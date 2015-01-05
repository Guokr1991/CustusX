/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/


/*
 * sscXmlOptionItem.cpp
 *
 *  Created on: May 28, 2010
 *      Author: christiana
 */
#include "cxXmlOptionItem.h"

#include <map>
#include <iostream>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDomElement>
#include <QStringList>
#include <QMutex>
#include "cxLogger.h"
#include "cxTypeConversions.h"

namespace cx
{

/**Helper class for reusing opened documents instead of creating new instances to them.
 *
 */
class SharedDocuments
{
public:
	static SharedDocuments* getInstance()
	{
		if (!mInstance)
			mInstance = new SharedDocuments();
		return mInstance;
	}

	/** read the document given by filename from disk,
	  * but use a cached document if already loaded.
	  */
	QDomDocument loadDocument(const QString& filename)
	{
		QDomDocument retval;
		retval = this->getDocument(filename);
		if (retval.isNull())
		{
			retval = this->readFromFile(filename);
			this->addDocument(filename, retval);
		}
		return retval;
	}

private:
	SharedDocuments() : mDocumentMutex(QMutex::Recursive) {}

	QDomDocument getDocument(const QString& filename)
	{
		QMutexLocker lock(&mDocumentMutex);
		DocumentMap::iterator iter = mDocuments.find(filename);
		// if filename found, attempt to retrieve document from node.
		if (iter != mDocuments.end())
		{
			return iter->second.ownerDocument();
		}
		return QDomDocument(); // null node
	}

	void addDocument(const QString& filename, QDomDocument document)
	{
		QMutexLocker lock(&mDocumentMutex);
		mDocuments[filename] = document.documentElement();
	}

	QDomDocument readFromFile(QString filename)
	{
		QFile file(filename);
		if (!file.open(QIODevice::ReadOnly))
		{
			QDomDocument doc;
			doc.appendChild(doc.createElement("root"));
			return doc;
		}

		QDomDocument loadedDoc;
		QString error;
		int line, col;
		if (!loadedDoc.setContent(&file, &error, &line, &col))
		{
			QString msg = QString("error setting xml content [%1,%2] %3").arg(line).arg(col).arg(error);
			reportWarning(msg);
		}
		file.close();
		return loadedDoc;
	}

	static SharedDocuments* mInstance;
	typedef std::map<QString, QDomElement> DocumentMap;
	QMutex mDocumentMutex;
	DocumentMap mDocuments; ///< QDomElement points to the documentElement. This acts as a weak_ptr.
};
///--------------------------------------------------------
SharedDocuments* SharedDocuments::mInstance = NULL;
///--------------------------------------------------------

///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------

XmlOptionItem::XmlOptionItem(const QString& uid, QDomElement root) :
	mUid(uid), mRoot(root)
{

}

QString XmlOptionItem::readValue(const QString& defval) const
{
	// read value is present
	QDomElement item = this->findElemFromUid(mUid, mRoot);
	if (!item.isNull() && item.hasAttribute("value"))
	{
		return item.attribute("value");
	}
	return defval;
}

void XmlOptionItem::writeValue(const QString& val)
{
	if (mRoot.isNull())
		return;
	QDomElement item = findElemFromUid(mUid, mRoot);
	// create option if not present
	if (item.isNull())
	{
		item = mRoot.ownerDocument().createElement("option");
		item.setAttribute("id", mUid);
		mRoot.appendChild(item);
	}
	item.setAttribute("value", val);
}

QDomElement XmlOptionItem::findElemFromUid(const QString& uid, QDomNode root) const
{
	QDomNodeList settings = root.childNodes();
	for (int i = 0; i < settings.size(); ++i)
	{
		QDomElement item = settings.item(i).toElement();
		if (item.attribute("id") == uid)
			return item;
	}
	return QDomElement();
}

/// -------------------------------------------------------
/// -------------------------------------------------------
/// -------------------------------------------------------

XmlOptionFile XmlOptionFile::createNull()
{
	XmlOptionFile retval;
	retval.mDocument = QDomDocument();
	retval.mCurrentElement = QDomElement();
	return retval;
}

XmlOptionFile::XmlOptionFile()
{
	mDocument.appendChild(mDocument.createElement("root"));
	mCurrentElement = mDocument.documentElement();
}

XmlOptionFile::XmlOptionFile(QString filename) :
	mFilename(filename)
{
	mDocument = SharedDocuments::getInstance()->loadDocument(filename);

	mCurrentElement = mDocument.documentElement();

	if (mCurrentElement.isNull())
	{
		mDocument.appendChild(mDocument.createElement("root"));
		mCurrentElement = mDocument.documentElement();
	}
}

XmlOptionFile::~XmlOptionFile()
{
}

QString XmlOptionFile::getFileName()
{
	return mFilename;
}

bool XmlOptionFile::isNull() const
{
	if(mCurrentElement.isNull() || mDocument.isNull())
		return true;
	return false;
}

XmlOptionFile XmlOptionFile::root() const
{
	XmlOptionFile retval = *this;
	retval.mCurrentElement = mDocument.documentElement();
	return retval;
}

XmlOptionFile XmlOptionFile::descend(QString element) const
{
	XmlOptionFile retval = *this;
	retval.mCurrentElement = retval.getElement(element);
	return retval;
}

XmlOptionFile XmlOptionFile::descend(QString element, QString attributeName, QString attributeValue) const
{
	XmlOptionFile retval = this->tryDescend(element, attributeName, attributeValue);
	if (!retval.getDocument().isNull())
		return retval;

	// create a new element if not found
	retval = *this;
	QDomElement current = retval.getDocument().createElement(element);
	current.setAttribute(attributeName, attributeValue);
	retval.mCurrentElement.appendChild(current);
	retval.mCurrentElement = current;
	return retval;
}

XmlOptionFile XmlOptionFile::tryDescend(QString element, QString attributeName, QString attributeValue) const
{
	XmlOptionFile retval = *this;

	QDomNodeList presetNodeList = retval.getElement().elementsByTagName(element);
	for (int i = 0; i < presetNodeList.count(); ++i)
	{
		QDomElement current = presetNodeList.item(i).toElement();
		QString name = current.attribute(attributeName);
		if (attributeValue == name)
		{
			retval.mCurrentElement = current;
			return retval;
		}
	}

	return XmlOptionFile::createNull();
}

XmlOptionFile XmlOptionFile::ascend() const
{
	XmlOptionFile retval = *this;
	retval.mCurrentElement = mCurrentElement.parentNode().toElement();
	if (retval.mCurrentElement.isNull())
		return *this;
	return retval;
}

/**return an element child of parent. Create if not existing.
 */
QDomElement XmlOptionFile::safeGetElement(QDomElement parent, QString childName)
{
	QDomElement child = parent.namedItem(childName).toElement();

	if (child.isNull())
	{
		child = mDocument.createElement(childName);
		parent.appendChild(child);
	}

	return child;
}

void XmlOptionFile::printDocument()
{
	QTextStream stream(stdout);
	stream << "\n" << mDocument.toString(4) << "\n";
}

void XmlOptionFile::printElement()
{
	QTextStream stream(stdout);
	stream << "\n";
	mCurrentElement.save(stream, 4);
	stream << "\n";
}

QDomDocument XmlOptionFile::getDocument()
{
	return mDocument;
}

QDomElement XmlOptionFile::getElement()
{
	return mCurrentElement;
}

QDomElement XmlOptionFile::getElement(QString level1)
{
	QDomElement elem1 = this->safeGetElement(mCurrentElement, level1);
	return elem1;
}

QDomElement XmlOptionFile::getElement(QString level1, QString level2)
{
	QDomElement elem1 = this->safeGetElement(mCurrentElement, level1);
	QDomElement elem2 = this->safeGetElement(elem1, level2);
	return elem2;
}

void XmlOptionFile::removeChildren()
{
	while (mCurrentElement.hasChildNodes())
		mCurrentElement.removeChild(mCurrentElement.firstChild());
}

void XmlOptionFile::deleteNode()
{
	QDomNode parentNode = mCurrentElement.parentNode();
	parentNode.removeChild(mCurrentElement);
	mCurrentElement = QDomElement();// Create null element
}

void XmlOptionFile::save()
{
	if (mFilename.isEmpty())
	{
		reportWarning("XmlOptionFile::save() No file name");
		return; //Don't do anything if on filename isn't supplied
	}

	QString path = QFileInfo(mFilename).absolutePath();
	QDir().mkpath(path);
	QFile file(mFilename);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		QTextStream stream(&file);
		stream << mDocument.toString(4);
		file.close();
//		report("Created " + file.fileName());
	}
	else
	{
		reportError("XmlOptionFile::save() Could not open " + file.fileName() + " Error: "
			+ file.errorString());
	}
}


} // namespace cx
