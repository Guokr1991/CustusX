/*
 * sscStringDataAdapterXml.h
 *
 *  Created on: Jun 27, 2010
 *      Author: christiana
 */

#ifndef SSCSTRINGDATAADAPTERXML_H_
#define SSCSTRINGDATAADAPTERXML_H_

#include <QDomElement>
#include <QStringList>
#include "sscStringDataAdapter.h"
#include "sscXmlOptionItem.h"

namespace ssc
{

typedef boost::shared_ptr<class StringDataAdapterXml> StringDataAdapterXmlPtr;

/** Represents one option of the string type.
 *  The data are stored within a xml document.
 *
 *  The option node has this layout:
 *   <option id="Processor" value="3.14"/>
 */
class StringDataAdapterXml: public StringDataAdapter
{
Q_OBJECT
public:
	/** Make sure one given option exists witin root.
	 * If not present, fill inn the input defaults.
	 */
	static StringDataAdapterXmlPtr initialize(const QString& uid, QString name, QString help, QString value,
		QStringList range, QDomNode root);

public:
	// inherited interface
	virtual QString getValueName() const;///< name of data entity. Used for display to user.
	virtual bool setValue(const QString& value); ///< set the data value.
	virtual QString getValue() const; ///< get the data value.
	virtual QString getHelp() const; ///< return a descriptive help string for the data, used for example as a tool tip.
	virtual QStringList getValueRange() const; /// range of value. Use if data is constrained to a set.
	virtual QString convertInternal2Display(QString internal); ///< conversion from internal value to display value
	virtual void setDisplayNames(std::map<QString, QString> names);

public:
	QString getUid() const;

signals:
	void valueWasSet(); /// emitted when the value is set using setValue() (similar to changed(), but more constrained)

private:
	QString mName;
	QString mUid;
	QString mHelp;
	QString mValue;
	QStringList mRange;
	XmlOptionItem mStore;
	std::map<QString, QString> mDisplayNames;
};

// --------------------------------------------------------
// --------------------------------------------------------


} // namespace ssc

#endif /* SSCSTRINGDATAADAPTERXML_H_ */
