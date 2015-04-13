#ifndef CXMAINWINDOWACTIONS_H
#define CXMAINWINDOWACTIONS_H

#include "cxGuiExport.h"

#include <QObject>
#include <map>
#include <set>
#include "boost/shared_ptr.hpp"

class QAction;
class QMenu;
class QActionGroup;
class QDockWidget;
class QScrollArea;

namespace cx
{
typedef boost::shared_ptr<class VisServices> VisServicesPtr;
typedef boost::shared_ptr<class ProcessWrapper> ProcessWrapperPtr;

/**
 * Contains the actions previously located inside the
 * MainWindow. Intended to be used from MainWindow.
 *
 * \ingroup cx_gui
 */
class cxGui_EXPORT MainWindowActions : public QObject
{
	Q_OBJECT
public:
	MainWindowActions(VisServicesPtr services, QWidget* parent);
	QAction* getAction(QString uid);

private slots:
	// File menu
	void newPatientSlot(); ///< Create new patient with directory structure
	void loadPatientFileSlot();///< Load all application data from XML file
	void savePatientFileSlot();///< Save all application data to XML file
	void clearPatientSlot();///< clear current patient (debug)
	//data menu
	void exportDataSlot();
	void importDataSlot(); ///< loads data(images) into the datamanager
	void deleteDataSlot(); ///< deletes data(image) from the patient

	void togglePointPickerActionSlot();
	void updatePointPickerActionSlot();

	//tool menu
	void configureSlot(); ///< lets the user choose which configuration files to use for the navigation

	// help
	void onGotoDocumentation();

	// navigation
	void centerToImageCenterSlot();
	void centerToTooltipSlot();

	void updateTrackingActionSlot();
	void toggleTrackingSlot();
	void toggleStreamingSlot();
	void updateStreamingActionSlot();

	void shootScreen();
	void shootWindow();
	void recordFullscreen();

	void onStartLogConsole();

private:
	VisServicesPtr mServices;
	void createActions();

	std::map<QString, QAction*> mActions;

	QAction* mShowPointPickerAction;
	QAction* mTrackingToolsAction; ///< action for asking the navigation system to start/stop tracking
	QAction* mStartStreamingAction; ///< start streaming of the default RT source.

	QString mLastImportDataFolder;
	ProcessWrapperPtr mLocalVideoServerProcess;

	void createPatientActions();
	void createTrackingActions();
	QString getExistingSessionFolder();
	QWidget* parentWidget();
	void saveScreenShot(QPixmap pixmap, QString id="");
	void saveScreenShotThreaded(QImage pixmap, QString filename);

	template <class T>
	QAction* createAction(QString uid, QString text, QIcon icon,
											 QKeySequence shortcut, QString help,
											 T triggerSlot);
};

} // namespace cx


#endif // CXMAINWINDOWACTIONS_H
