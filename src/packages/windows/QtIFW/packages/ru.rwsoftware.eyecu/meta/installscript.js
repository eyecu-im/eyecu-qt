/**************************************************************************
**
** Copyright (C) 2015 Road Works Software
** Contact: http://www.rwsoftware.ru
**
** This file is part of eyeCU project.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
**************************************************************************/

function Component()
{
    if (installer.isInstaller() || installer.isUpdater())
        component.loaded.connect(this, Component.prototype.installerLoaded);
}

Component.prototype.createOperations = function()
{
	QMessageBox.information("trace", "createOperations()", "startMenu="+installer.value("StartMenu"), QMessageBox.Ok);
    component.createOperations();

    if (installer.value("os") == "win") {
    	startMenuDir=installer.value("StartMenuDir");
    	QMessageBox.information("trace", "startMenuDir", startMenuDir, QMessageBox.Ok);
	    if (installer.isUpdater()) {
    		QMessageBox.information("trace", "isUpdater()", "Reading current values", QMessageBox.Ok);
			allUsers = (startMenuDir=="" || startMenuDir.indexOf(installer.value("AllUsersStartMenuProgramsPath"))==0);
			QMessageBox.information("trace", "allUsers", allUsers, QMessageBox.Ok);
			key = allUsers?"HKEY_LOCAL_MACHINE":"HKEY_CURRENT_USER";
    		startMenu=installer.value(key+"/RoadWorksSoftware/eyeCU/StartMenu", true);
	    	desktop=installer.value(key+"HKEY_CURRENT_USER/RoadWorksSoftware/eyeCU/Desktop", false);
//    		allUsers=installer.gainAdminRights();
//    		allUsers=installer.value("HKEY_CURRENT_USER/RoadWorksSoftware/eyeCU/AllUsers", allUsers);
			installer.setValue("StartMenu", startMenu);
			installer.setValue("Desktop", desktop);
//			installer.setValue("AllUsers", allUsers);
		} else {
    		allUsers=installer.value("AllUsers");	
			if (allUsers)
				startMenuDir = startMenuDir.replace(installer.value("UserStartMenuProgramsPath"), installer.value("AllUsersStartMenuProgramsPath"));
			startMenuDir=installer.value("StartMenuDir");
			QMessageBox.information("trace", "StartMenuDir(after)", startMenuDir, QMessageBox.Ok);
	    	startMenu=installer.value("StartMenu");
			installer.setValue("StartMenuDir", startMenuDir);
	        desktop=installer.value("Desktop", false);
		}

		scope=allUsers?"SystemScope":"UserScope";
		QMessageBox.information("trace", "Values", "startMenu="+startMenu+"; desktop="+desktop+"; allUsers="+allUsers+"; scope="+scope, QMessageBox.Ok);
		if (startMenu) {
			QMessageBox.information("trace", "Trace", "Adding operations...", QMessageBox.Ok);
   	        component.addOperation("CreateShortcut", "@TargetDir@/eyecu.exe", startMenuDir+"\\eyeCU.lnk", "workingDirectory=@TargetDir@");
			component.addOperation("CreateShortcut", "@TargetDir@/maintain.exe", startMenuDir+"\\Maintain eyeCU.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=162");
        }
        QMessageBox.information("trace", "Adding operations...", "StartMenu="+startMenu+"; AllUsers="+allUsers, QMessageBox.Ok);
        component.addOperation("GlobalConfig", scope, "RoadWorksSoftware", "eyeCU", "StartMenu", startMenu);
        component.addOperation("GlobalConfig", scope, "RoadWorksSoftware", "eyeCU", "AllUsers", allUsers);

		if (desktop)
       	    component.addOperation("CreateShortcut", "@TargetDir@/eyecu.exe", "@DesktopDir@\\eyeCU.lnk", "workingDirectory=@TargetDir@");
       	QMessageBox.information("trace", "Adding operation...", "Desktop="+desktop, QMessageBox.Ok);
       	component.addOperation("GlobalConfig", scope, "RoadWorksSoftware", "eyeCU", "Desktop", desktop);       	    
    }
    if (installer.value("Portable"))
        installer.addOperation("Mkdir", "@TargetDir@/eyecu");
	QMessageBox.information("trace", "createOperations()", "Done!", QMessageBox.Ok);
}

Component.prototype.installerLoaded = function () {
//    installer.setSharedFlag("StartMenu", true);
	QMessageBox.information("trace", "installerLoaded()", "startMenu="+installer.value("StartMenu"), QMessageBox.Ok);

	if (installer.isInstaller()) {
		installer.setValue("StartMenu", false);
    	if (installer.addWizardPage(component, "InstallationTypePage", QInstaller.TargetDirectory)) {
        	var widget = gui.pageWidgetByObjectName("DynamicInstallationTypePage");
        	if (widget != null) {
				widget.windowTitle = "Select installation type";
	            widget.installAllUsers.toggled.connect(this, Component.prototype.installAllUsersToggled);
    	        widget.installMeOnly.toggled.connect(this, Component.prototype.installMeOnlyToggled);
				if (installer.gainAdminRights())
				{
					widget.installAllUsers.checked = true;
					Component.prototype.installAllUsersToggled(true);
				}
				else
				{
					widget.installMeOnly.checked = true;
					widget.installAllUsers.enabled = false;
					Component.prototype.installMeOnlyToggled(true);
				}
	        }
	    }

	    if (installer.addWizardPageItem(component, "AdditionalOptionsPageForm", QInstaller.StartMenuSelection)) {
    	    var widget = gui.pageWidgetByObjectName("StartMenuDirectoryPage").AdditionalOptionsPageForm;
        	if (widget != null) {
            	widget.desktopCheckBox.toggled.connect(this, Component.prototype.desktopToggled);
	            widget.startMenuCheckBox.toggled.connect(this, Component.prototype.startMenuToggled);
    	        widget.portableCheckBox.toggled.connect(this, Component.prototype.portableToggled);
        	    widget.startMenuCheckBox.checked=true;
        	}
	    }
    }
}

Component.prototype.desktopToggled = function (checked) {
    installer.setValue("Desktop", checked);
}

Component.prototype.startMenuToggled = function (checked) {
	QMessageBox.information("trace", "startMenuToggled()", "checked="+checked, QMessageBox.Ok);
    installer.setValue("StartMenu", checked);
    var page = gui.pageWidgetByObjectName("StartMenuDirectoryPage");
    if (page != null)
    {
        var lineEdit = page.LineEdit;
        if (lineEdit == null)
           lineEdit = page.StartMenuPathLineEdit; // Since QtIFW 2.0
        
        if (lineEdit != null)
    	    lineEdit.setEnabled(checked);
    }
}

Component.prototype.portableToggled = function (checked) {
    installer.setValue("Portable", checked);
}

Component.prototype.installAllUsersToggled = function (checked) {
    if (checked) {        
        installer.setValue("TargetDir", "@ApplicationsDir@/Road Works Software/eyecu");
        if (installer.value("os") === "win")
           installer.setValue("AllUsers", true);
    }
}

Component.prototype.installMeOnlyToggled = function (checked) {
    if (checked) {
        installer.setValue("TargetDir", "@HomeDir@/Road Works Software/eyecu");
        if (installer.value("os") === "win")
           installer.setValue("AllUsers", false);
    }
}