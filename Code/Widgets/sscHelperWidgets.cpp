/*
 * sscHelperWidgets.cpp
 *
 *  Created on: May 18, 2010
 *      Author: christiana
 */
#include "sscHelperWidgets.h"

#include "sscDoubleWidgets.h"
#include "sscLabeledComboBoxWidget.h"
#include "sscCheckBoxWidget.h"

namespace ssc
{

QWidget* createDataWidget(QWidget* parent, DataAdapterPtr data, QGridLayout* gridLayout, int row)
{
	StringDataAdapterPtr str = boost::shared_dynamic_cast<ssc::StringDataAdapter>(data);
	if (str)
	{
		return new ssc::LabeledComboBoxWidget(parent, str, gridLayout, row);
	}

	DoubleDataAdapterPtr dbl = boost::shared_dynamic_cast<ssc::DoubleDataAdapter>(data);
	if (dbl)
	{
		return new ssc::SliderGroupWidget(parent, dbl, gridLayout, row);
	}

	BoolDataAdapterPtr bl = boost::shared_dynamic_cast<ssc::BoolDataAdapter>(data);
	if (bl)
	{
		return new ssc::CheckBoxWidget(parent, bl, gridLayout, row);
	}

	return NULL;
}

}
