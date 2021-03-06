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
#include "cxCustomMetaImage.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include "cxLogger.h"

#include "cxTypeConversions.h"

namespace cx
{

CustomMetaImage::CustomMetaImage(QString filename) :
    mFilename(filename)
{}

QString CustomMetaImage::readKey(QString key)
{
	QFile file(mFilename);

	QString line;
	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream t(&file);
		while (!t.atEnd())
		{
			line.clear();
			line = t.readLine();
			// do something with the line
			if (line.startsWith(key, Qt::CaseInsensitive))
			{
				QStringList list = line.split("=", QString::SkipEmptyParts);
				if (list.size() >= 2)
				{
					list = list.mid(1);
					return list.join("=");
				}
			}
		}
		file.close();
	}

	return "";
}

QString CustomMetaImage::readModality()
{
	QString mod = this->readKey("Modality");

	if (mod.contains("CT", Qt::CaseInsensitive))
		return "CT";
	if (mod.contains("MR", Qt::CaseInsensitive))
		return "MR";
	if (mod.contains("US", Qt::CaseInsensitive))
		return "US";
	if (mod.contains("OTHER", Qt::CaseInsensitive))
		return "OTHER";
	mod = mod.remove("MET_MOD_");
	return mod;
}

QString CustomMetaImage::readImageType()
{
	return this->readKey("ImageType3");
}

/** Remove all lines starting with a key from data.
  *
  */
void CustomMetaImage::remove(QStringList* data, QStringList keys)
{
	QRegExp regexp(QString("(^%1)").arg(keys.join("|^")));
	QStringList removeThese = data->filter(regexp);
	for (int i=0; i<removeThese.size(); ++i)
		data->removeAll(removeThese[i]);
}

/** Append key, value pair to data.
  * The line is added last, but BEFORE the "ElementDataFile" key,
  * which is required to be last.
  *
  */
void CustomMetaImage::append(QStringList* data, QString key, QString value)
{
	// find index of ElementDataFile - this is the last element according to MHD standard (but we might have appended something else after it).
    int last = data->lastIndexOf(QRegExp("^ElementDataFile.*"));
	data->insert(last, QString("%1 = %2").arg(key).arg(value));
}

void CustomMetaImage::setKey(QString key, QString value)
{
	QFile file(mFilename);

	if (!file.open(QIODevice::ReadWrite))
	{
	  reportError("Failed to open file " + mFilename + ".");
	  return;
	}

	QStringList data = QTextStream(&file).readAll().split("\n");

	this->remove(&data, QStringList()<<key);
	this->append(&data, key, value);

	file.resize(0);
	file.write(data.join("\n").toLatin1());
}

void CustomMetaImage::setModality(QString value)
{
    QStringList valid_values;
    valid_values << "CT" << "MR" << "NM" << "US" << "OTHER" << "UNKNOWN";
    if(!valid_values.contains(value, Qt::CaseInsensitive))
        value = "UNKNOWN";

	this->setKey("Modality", value);
}

void CustomMetaImage::setImageType(QString value)
{
	if (value.isEmpty())
		return;
	this->setKey("ImageType3", value);
}


Transform3D CustomMetaImage::readTransform()
{
  //read the specific TransformMatrix-tag from the header
  Vector3D p_r(0, 0, 0);
  Vector3D e_x(1, 0, 0);
  Vector3D e_y(0, 1, 0);
  Vector3D e_z(0, 0, 1);

  QFile file(mFilename);

  QString line;
  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream t(&file);
    while (!t.atEnd())
    {
      line.clear();
      line = t.readLine();
      // do something with the line
      if (line.startsWith("Position", Qt::CaseInsensitive) || line.startsWith("Offset", Qt::CaseInsensitive))
      {
        QStringList list = line.split(" ", QString::SkipEmptyParts);
        if (list.size()>=5)
          p_r = Vector3D(list[2].toDouble(), list[3].toDouble(), list[4].toDouble());
      }
      else if (line.startsWith("TransformMatrix", Qt::CaseInsensitive) || line.startsWith("Orientation",
          Qt::CaseInsensitive))
      {
        QStringList list = line.split(" ", QString::SkipEmptyParts);

        if (list.size()>=8)
        {
          e_x = Vector3D(list[2].toDouble(), list[3].toDouble(), list[4].toDouble());
          e_y = Vector3D(list[5].toDouble(), list[6].toDouble(), list[7].toDouble());
          e_z = cross(e_x, e_y);
        }
      }
    }
    file.close();
  }

  Transform3D rMd = Transform3D::Identity();

  // add rotational part
  for (unsigned i = 0; i < 3; ++i)
  {
    rMd(i,0) = e_x[i];
    rMd(i,1) = e_y[i];
    rMd(i,2) = e_z[i];
  }


  // add translational part
  rMd(0,3) = p_r[0];
  rMd(1,3) = p_r[1];
  rMd(2,3) = p_r[2];
  return rMd;
}

void CustomMetaImage::setTransform(const Transform3D M)
{
  QFile file(mFilename);

  if (!file.open(QIODevice::ReadWrite))
  {
    reportError("Failed to open file " + mFilename + ".");
    return;
  }

  QStringList data = QTextStream(&file).readAll().split("\n");

  this->remove(&data, QStringList()<<"TransformMatrix"<<"Offset"<<"Position"<<"Orientation");

  int dim = 3; // hardcoded - will fail for 2d images
  std::stringstream tmList;
  for (int c=0; c<dim; ++c)
    for (int r=0; r<dim; ++r)
      tmList << " " << M(r,c);
  this->append(&data, "TransformMatrix", qstring_cast(tmList.str()));

  std::stringstream posList;
  for (int r=0; r<dim; ++r)
    posList << " " << M(r,3);
  this->append(&data, "Offset", qstring_cast(posList.str()));

  file.resize(0);
  file.write(data.join("\n").toLatin1());
}

}
