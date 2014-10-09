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

#include "cxHelpWidget.h"

#include "boost/bind.hpp"
#include "boost/function.hpp"
#include <QHelpEngine>
#include <QSplitter>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QTabWidget>

#include "cxTypeConversions.h"
#include "cxHelpEngine.h"
#include "cxHelpBrowser.h"
#include "cxHelpSearchWidget.h"
#include "cxHelpIndexWidget.h"
#include "cxSettings.h"

namespace cx
{

HelpWidget::HelpWidget(HelpEnginePtr engine, QWidget* parent) :
	BaseWidget(parent, "HelpWidget", "Help"),
	mVerticalLayout(NULL),
	mTabWidget(NULL),
	mEngine(engine)
{
}

void HelpWidget::setup()
{
	if (mVerticalLayout)
		return;

	mVerticalLayout = new QVBoxLayout(this);
	mVerticalLayout->setMargin(0);
	mVerticalLayout->setSpacing(0);
	this->setLayout(mVerticalLayout);
	mTabWidget = new QTabWidget(this);
	mTabWidget->setElideMode(Qt::ElideRight);

	QSplitter *splitter = new QSplitter(Qt::Horizontal);
	mSplitter = splitter;

	HelpBrowser *browser = new HelpBrowser(this, mEngine);
	connect(this, &HelpWidget::requestShowLink,
			browser, &HelpBrowser::setSource);

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	//	buttonLayout->setMargin(0);
	mVerticalLayout->addLayout(buttonLayout);

	splitter->insertWidget(0, mTabWidget);
	splitter->insertWidget(1, browser);
	splitter->setStretchFactor(1, 1);
	mVerticalLayout->addWidget(splitter);

	this->addContentWidget(mTabWidget, buttonLayout);
	this->addSearchWidget(mTabWidget, buttonLayout);
	this->addIndexWidget(mTabWidget, buttonLayout);

	this->addToggleTabWidgetButton(buttonLayout);
	buttonLayout->addStretch();

	browser->showHelpForKeyword("mainpage_overview");

	bool navVis = settings()->value("org.custusx.help/navigationVisible").toBool();
//	mTabWidget->hide();
	mTabWidget->setVisible(navVis);
}

HelpWidget::~HelpWidget()
{}

QString HelpWidget::defaultWhatsThis() const
{
	return "<html>"
			"<h3>Experimental help</h3>"
			"<p></p>"
			"<p><i></i></p>"
			"</html>";
}

void HelpWidget::addContentWidget(QTabWidget* tabWidget, QBoxLayout* buttonLayout)
{
	QHelpContentWidget* contentWidget = mEngine->engine()->contentWidget();
	tabWidget->addTab(contentWidget, "contents");

	boost::function<void()> f = boost::bind(&QHelpContentWidget::expandToDepth, contentWidget, 2);
	connect(mEngine->engine()->contentModel(), &QHelpContentModel::contentsCreated, f);
	contentWidget->expandToDepth(2); // in case contents have been created

	connect(mEngine->engine()->contentWidget(), &QHelpContentWidget::linkActivated,
			this, &HelpWidget::requestShowLink);
}

void HelpWidget::addToggleTabWidgetButton(QBoxLayout* buttonLayout)
{
	QAction* action = this->createAction(this,
										   QIcon(":/icons/open_icon_library/view-list-tree.png"),
										   "Toggle show navigation controls", "",
										   SLOT(toggleShowNavigationControls()),
										   NULL);
	action->setCheckable(true);
	CXSmallToolButton* button = new CXSmallToolButton();
	button->setDefaultAction(action);
	buttonLayout->addWidget(button);
	mShowNavigationControlsAction = action;
}

void HelpWidget::addIndexWidget(QTabWidget* tabWidget, QBoxLayout* buttonLayout)
{
	mIndexWidget = new HelpIndexWidget(mEngine, this);
	tabWidget->addTab(mIndexWidget, "index");

	connect(mIndexWidget, &HelpIndexWidget::requestShowLink,
			this, &HelpWidget::requestShowLink);
}

void HelpWidget::addSearchWidget(QTabWidget* tabWidget, QBoxLayout* buttonLayout)
{
	mSearchWidget = new HelpSearchWidget(mEngine, this);
	tabWidget->addTab(mSearchWidget, "search");
	connect(mSearchWidget, &HelpSearchWidget::requestShowLink,
			this, &HelpWidget::requestShowLink);
}

void HelpWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	this->setModified();
}

void HelpWidget::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);
}

void HelpWidget::prePaintEvent()
{
	this->setup();
}

void HelpWidget::toggleShowNavigationControls()
{
	if (mTabWidget->isVisible())
		mTabWidget->hide();
	else
	{
		mTabWidget->show();

		QList<int> sizes = mSplitter->sizes();
		if (sizes[0]==0)
		{
			sizes[0] = sizes[1]*1/3;
			sizes[1] = sizes[1]*2/3;
			mSplitter->setSizes(sizes);
		}

	}
	settings()->setValue("org.custusx.help/navigationVisible", mTabWidget->isVisible());
}

}//end namespace cx
