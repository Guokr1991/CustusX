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
#ifndef CXLOGICMANAGER_H_
#define CXLOGICMANAGER_H_

#include "cxLogicManagerExport.h"

#include <boost/shared_ptr.hpp>
#include <QObject>
#include "cxForwardDeclarations.h"
#include "cxLegacySingletons.h"

class ctkPluginContext;

namespace cx
{
/**
* \file
* \addtogroup cx_logic
* @{
*/

typedef boost::shared_ptr<class PluginFrameworkManager> PluginFrameworkManagerPtr;

typedef boost::shared_ptr<class ApplicationComponent> ApplicationComponentPtr;
/**
 * Class holding anything that can be created/destroyed and exist meanwhile.
 */
class ApplicationComponent
{
public:
	virtual ~ApplicationComponent() {}

	virtual void create() = 0;
	virtual bool exists() const = 0;
	virtual void destroy() = 0;
};

/** \brief Control the custusx backend.
 *  \ingroup cx_logic
 *
 *
 */
class cxLogicManager_EXPORT LogicManager : public QObject
{
	Q_OBJECT
public:
  static LogicManager* getInstance();

  /**
	* Initialize the manager, including all services (calls initializeServices() ).
	*/
  static void initialize(ApplicationComponentPtr component=ApplicationComponentPtr());
  /**
	* Shutdown the manager, including all services (calls shutdownServices() ).
	*/
  static void shutdown();

  /**
   * Do a complete restart of the system:
   *   Shutdown all components,
   *   Then restart all of them.
   * This is done asynchronolusly.
   */
  void restartWithNewProfile(QString uid);

  PluginFrameworkManagerPtr getPluginFramework();
  ctkPluginContext* getPluginContext();

  TrackingServicePtr getTrackingService();
  SpaceProviderPtr getSpaceProvider();
  PatientModelServicePtr getPatientModelService();
  VideoServicePtr getVideoService();
  StateServicePtr getStateService();
  ViewServicePtr getViewService();
  SessionStorageServicePtr getSessionStorageService();

private slots:
  void onRestartWithNewProfile(QString uid);

private:
  /**
	* Initialize all system services, resources and other static objects.
	*/
  void initializeServices();
  /**
   * Set an application component, intended to encapsulate the application's
   * main window or similar. Must be called after initialize.
   *
   * Component will be created here and destroyed in shutdown.
   */
  void setApplicationComponent(ApplicationComponentPtr component);
  /**
	* Shutdown all system services, resources and other static objects.
	*
	* Deallocate all global resources.
	* Assumes MainWindow already has been destroyed and the mainloop is exited.
	*/
  void shutdownServices();

  template<class T>
  void shutdownService(boost::shared_ptr<T>& service, QString name);

  static LogicManager* mInstance;
  static void setInstance(LogicManager* instance);

  LogicManager();
	virtual ~LogicManager();

	LogicManager(LogicManager const&); // not implemented
	LogicManager& operator=(LogicManager const&); // not implemented

// services:
	SpaceProviderPtr mSpaceProvider;
	PatientModelServicePtr mPatientModelService;
	TrackingServicePtr mTrackingService;
	VideoServicePtr mVideoService;
	StateServicePtr mStateService;
	ViewServicePtr mViewService;
	SessionStorageServicePtr mSessionStorageService;

	PluginFrameworkManagerPtr mPluginFramework;
	ApplicationComponentPtr mComponent;
};

cxLogicManager_EXPORT LogicManager* logicManager(); // access the singleton

/**
* @}
*/
}

#endif /* CXLOGICMANAGER_H_ */
