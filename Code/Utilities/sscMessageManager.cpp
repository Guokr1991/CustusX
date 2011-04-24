#include "sscMessageManager.h"

#include <iostream>
#include "boost/shared_ptr.hpp"
#include <QString>
#include <QMutex>
#include <QSound>
#include <QDir>
#include <QTextStream>
#include "sscTypeConversions.h"
#include "sscDefinitionStrings.h"
#include "sscTime.h"

namespace ssc
{


Message::Message(QString text, MESSAGE_LEVEL messageLevel, int timeoutTime) :
  mText(text),
  mMessageLevel(messageLevel),
  mTimeoutTime(timeoutTime),
  mTimeStamp(QDateTime::currentDateTime())
{};

Message::~Message(){};

QString Message::getPrintableMessage()
{
  QString message("");
  message.append(QString("["));
  message.append(mTimeStamp.toString("hh:mm:ss:zzz"));
  message.append(QString("] "));
  message.append(QString("["));
  message.append(qstring_cast(mMessageLevel));
  message.append(QString("] "));
  message.append(mText);

  return message;
}

MESSAGE_LEVEL Message::getMessageLevel()
{
  return mMessageLevel;
}

QString Message::getText()
{
  return mText;
}

QDateTime* Message::getTimeStamp()
{
  return &mTimeStamp;
}
int Message::getTimeout()
{
  return mTimeoutTime;
}
// --------------------------------------------------------
// --------------------------------------------------------


namespace
{
// --------------------------------------------------------


/** streambuf subclass: used to override either cout or cerr and
 * reroute text to both messagemanager and original stream.
 *
 */
class MyStreamBuf: public std::basic_streambuf<char, std::char_traits<char> >
{
public:
  MyStreamBuf(MESSAGE_LEVEL level) :
    mEnabledRedirect(true), mOrig(NULL), mMessageLevel(level)
  {
  }
  void setOriginal(std::streambuf* orig)
  {
    mOrig = orig;
  }
  virtual int_type overflow(int_type meta = traits_type::eof())
  {
    char single = traits_type::to_char_type(meta);
    if (mOrig) // send to original stream as well
      mOrig->sputc(single);

    if (mEnabledRedirect)
    {
      if (single == '\n')
      {
        QMutexLocker sentry(&mMutex);
        QString buffer = qstring_cast(mBuffer);
        mBuffer.clear();
        sentry.unlock();

        messageManager()->sendMessage(buffer, mMessageLevel, 0);
      }
      else
      {
        QMutexLocker sentry(&mMutex);
        mBuffer += single;
      }
    }
    return traits_type::not_eof(meta);
  }
  void setEnableRedirect(bool on)
  {
    mEnabledRedirect = on;
  }

  //    virtual int_type overflow(int_type meta=traits_type::eof());
private:
  bool mEnabledRedirect;
  QString mBuffer;
  std::streambuf* mOrig;
  MESSAGE_LEVEL mMessageLevel;
  QMutex mMutex;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
}

class SingleStreamerImpl
{
private:
  boost::shared_ptr<class MyStreamBuf> StreamBuf;
  std::streambuf *OrigBuf;
  std::ostream& mStream;
public:
  SingleStreamerImpl(std::ostream& str, MESSAGE_LEVEL level) :
    mStream(str)
  {
    StreamBuf.reset(new MyStreamBuf(level));
    OrigBuf = mStream.rdbuf(StreamBuf.get());
    StreamBuf->setOriginal(OrigBuf);
  }
  ~SingleStreamerImpl()
  {
    mStream.rdbuf(OrigBuf);
  }
  void setEnableRedirect(bool on)
  {
    StreamBuf->setEnableRedirect(on);
  }

};

MessageManager* MessageManager::mTheInstance = NULL;
MessageManager* messageManager() { return MessageManager::getInstance(); }


MessageManager::MessageManager() :
    mConsoleMutex(QMutex::Recursive),
    mAbsoluteLoggingFolderPath(""),
    mConsoleFile(new QFile(mAbsoluteLoggingFolderPath, this)),
    mConsoleStream(new QTextStream())
{
  typedef ssc::Message Message;
  qRegisterMetaType<Message>("Message");
}

MessageManager::~MessageManager()
{
  mConsoleFile->close();
  delete mConsoleStream;
}

MessageManager* MessageManager::getInstance()
{
  if(mTheInstance == NULL)
  {
    mTheInstance = new MessageManager();
    mTheInstance->initialize();
  }
  return mTheInstance;
}

void MessageManager::initialize()
{
  mCout.reset(new SingleStreamerImpl(std::cout, mlCOUT));
  mCerr.reset(new SingleStreamerImpl(std::cerr, mlCERR));
}

void MessageManager::destroyInstance()
{
  delete mTheInstance;
}

void MessageManager::setLoggingFolder(QString absoluteLoggingFolderPath)
{
  if(mAbsoluteLoggingFolderPath == absoluteLoggingFolderPath)
    return;

  QString filename("/ConsoleLog.txt");
  mAbsoluteLoggingFolderPath = absoluteLoggingFolderPath;
  QDir().mkdir(absoluteLoggingFolderPath);
  QString consoleFilePath = mAbsoluteLoggingFolderPath+filename;

  QMutexLocker sentry(&mConsoleMutex);
  if(mConsoleFile->isOpen())
  {
    mConsoleFile->close();

    // cannot delete file: this will delete previous logging of a patient that has
    // been saved and reopened. Better to keep data in their respective folders.

//    if(QFile::exists(consoleFilePath))
//      QFile::remove(consoleFilePath);
//
//    if(!mConsoleFile->copy(consoleFilePath))
//    {
//      sentry.unlock();
//      ssc::messageManager()->sendWarning("Could not copy to "+consoleFilePath);
//      sentry.relock();
//    }

  }
  else
  {
  }

  mConsoleFile->setFileName(consoleFilePath);
  sentry.unlock();
  this->openLogging(QFile::Append);
}

void MessageManager::setAudioSource(ssc::AudioPtr audioSource)
{
  mAudioSource = audioSource;
}

bool MessageManager::hasAudioSource() const
{
  return mAudioSource;
}

void MessageManager::sendInfo(QString info)
{
  this->sendMessage(info, mlINFO, 1500);
}

void MessageManager::sendSuccess(QString success, bool mute)
{
  this->sendMessage(success, mlSUCCESS, 1500);
  if(!mute)
    this->playSuccessSound();
}

void MessageManager::sendWarning(QString warning, bool mute)
{
  this->sendMessage(warning, mlWARNING, 3000);
  if(!mute)
    this->playWarningSound();
}

void MessageManager::sendError(QString error, bool mute)
{
  this->sendMessage(error, mlERROR, 0);
  if(!mute)
    this->playErrorSound();
}
  
void MessageManager::sendDebug(QString debug)
{
  this->sendMessage(debug, mlDEBUG, 0);
}

void MessageManager::sendMessage(QString text, MESSAGE_LEVEL messageLevel, int timeout)
{
  Message message(text, messageLevel, timeout);

  // send text to cout if it not already comes from that stream (or cerr)
  mCout->setEnableRedirect(false);
  if (messageLevel!=mlCOUT && messageLevel!=mlCERR)
    std::cout << message.getPrintableMessage() << std::endl;
  mCout->setEnableRedirect(true);

  QMutexLocker sentry(&mConsoleMutex);
  if(mConsoleStream->device())
  {
    (*mConsoleStream) << message.getPrintableMessage();
    (*mConsoleStream) << endl;
  }
  sentry.unlock();

  emit emittedMessage(message);
}

void MessageManager::playStartSound()
{
  if(this->hasAudioSource())
    mAudioSource->playStartSound();
}

void MessageManager::playStopSound()
{
  if(this->hasAudioSource())
    mAudioSource->playStopSound();
}

void MessageManager::playCancelSound()
{
  if(this->hasAudioSource())
    mAudioSource->playCancelSound();
}

void MessageManager::playSuccessSound()
{
  if(this->hasAudioSource())
    mAudioSource->playSuccessSound();
}

void MessageManager::playWarningSound()
{
  if(this->hasAudioSource())
    mAudioSource->playWarningSound();
}

void MessageManager::playErrorSound()
{
  if(this->hasAudioSource())
    mAudioSource->playErrorSound();
}

void MessageManager::playScreenShotSound()
{
  if(this->hasAudioSource())
    mAudioSource->playScreenShotSound();
}

void MessageManager::playSampleSound()
{
  if(this->hasAudioSource())
    mAudioSource->playSampleSound();
}

bool MessageManager::openLogging(QFile::OpenMode mode)
{
  QMutexLocker sentry(&mConsoleMutex);
  if(mConsoleFile->open(QFile::WriteOnly | mode))
  {
    mConsoleStream->setDevice(mConsoleFile);
    sentry.unlock();
    ssc::messageManager()->sendInfo("Console log file: "+mConsoleFile->fileName()+".");
  }
  else
  {
    sentry.unlock();
    ssc::messageManager()->sendWarning("Could not open "+mConsoleFile->fileName()+" for writing the console log.");
    return false;
  }
  return true;
}

} //End namespace ssc
