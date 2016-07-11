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

function Component() {
	QMessageBox.information("component.info", "Component()", "A", QMessageBox.Ok);
    if (installer.isInstaller() || installer.isUpdater()) {
	    QMessageBox.information("component.info", "Component()", "B", QMessageBox.Ok);
        component.loaded.connect(this, Component.prototype.loaded);
		installer.installationStarted.connect(this, Component.prototype.installationStarted);
	}
}

Component.prototype.createOperations = function() {
	component.createOperations();
	if (installer.value("os") == "win") {
		startMenuDir=installer.value("StartMenuDir");
		allUsers=installer.value("AllUsers")=="true";
   		startMenu = installer.value("StartMenu")=="true";
		desktop = installer.value("Desktop")=="true";

		scope=allUsers?"SystemScope":"UserScope";
		if (startMenu) {
			component.addOperation("CreateShortcut", "@TargetDir@/eyecu.exe", startMenuDir+"\\eyeCU.lnk", "workingDirectory=@TargetDir@");
			component.addOperation("CreateShortcut", "@TargetDir@/maintain.exe", startMenuDir+"\\Maintain eyeCU.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=162");
		}
		component.addOperation("GlobalConfig", scope, "Road Works Software", "eyeCU", "StartMenu", startMenu);
		component.addOperation("GlobalConfig", scope, "Road Works Software", "eyeCU", "AllUsers", allUsers);

		if (desktop)
			component.addOperation("CreateShortcut", "@TargetDir@/eyecu.exe", "@DesktopDir@\\eyeCU.lnk", "workingDirectory=@TargetDir@");
		component.addOperation("GlobalConfig", scope, "Road Works Software", "eyeCU", "Desktop", desktop);
	}
	if (installer.value("Portable")=="true")
		component.addOperation("Mkdir", "@TargetDir@/eyecu");
}

Component.prototype.beginInstallation = function() {
	var targetPath = installer.value("TargetDir") + "\\eyecu.exe";
	component.addStopProcessForUpdateRequest(targetPath);
	component.beginInstallation();
}

Component.prototype.loaded = function () {
	if (installer.isInstaller()) {
		installer.setValue("StartMenu", false);
		if (installer.addWizardPage(component, "InstallationTypePage", QInstaller.TargetDirectory)) {
			var widget = gui.pageWidgetByObjectName("DynamicInstallationTypePage");
			if (widget != null) {
				widget.windowTitle = "Select installation type";
				widget.installAllUsers.toggled.connect(this, Component.prototype.installAllUsersToggled);
				widget.installMeOnly.toggled.connect(this, Component.prototype.installMeOnlyToggled);
				widget.installAllUsers.checked = true;
				Component.prototype.installAllUsersToggled(true);
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

Component.prototype.installationStarted = function() {
	startMenuDir=installer.value("StartMenuDir");
	if (installer.isInstaller()) {
		if (installer.value("AllUsers")=="true")
			startMenuDir = startMenuDir.replace(installer.value("UserStartMenuProgramsPath"), installer.value("AllUsersStartMenuProgramsPath"));
		installer.setValue("StartMenuDir", startMenuDir);
	} else if (installer.isUpdater()) {
		allUsers = (startMenuDir=="" || startMenuDir.indexOf(installer.value("AllUsersStartMenuProgramsPath"))==0);
		key = allUsers?"HKEY_LOCAL_MACHINE":"HKEY_CURRENT_USER";
		startMenu = installer.value(key+"/Road Works Software/eyeCU/StartMenu", "true")=="true";
		desktop = installer.value(key+"/Road Works Software/eyeCU/Desktop", "false")=="true";
		installer.setValue("StartMenu", startMenu);
		installer.setValue("Desktop", desktop);
	}
}

Component.prototype.desktopToggled = function (checked) {
    installer.setValue("Desktop", checked);
}

Component.prototype.startMenuToggled = function (checked) {
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