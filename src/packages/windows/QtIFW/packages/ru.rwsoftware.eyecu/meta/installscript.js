/**************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Installer Framework.
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
**
** $QT_END_LICENSE$
**
**************************************************************************/

function Component()
{
    if (installer.isInstaller())
        component.loaded.connect(this, Component.prototype.installerLoaded);
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install eyeCU!
    component.createOperations();

    if (installer.value("os") === "win") {
	if (installer.sharedFlag("StartMenu")) {
            component.addOperation("CreateShortcut", "@TargetDir@/eyecu.exe", "@TargetMenu@/eyeCU.lnk", "workingDirectory=@TargetDir@");
            component.addOperation("CreateShortcut", "@TargetDir@/maintain.exe", "@TargetMenu@/Maintain eyeCU.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=32");
        }
	if (component.value("Desktop"))
            component.addOperation("CreateShortcut", "@TargetDir@/eyecu.exe", "@DesktopDir@/eyeCU.lnk", "workingDirectory=@TargetDir@");
    }
    if (component.value("Portable"))
        component.addOperation("Mkdir", "@TargetDir@/eyecu");
}

Component.prototype.installerLoaded = function () {
    installer.setSharedFlag("StartMenu", true);
    
//    if (installer.value("os") === "win")
//        installer.setValue("TargetDir", "@ApplicationsDir@/Road Works Software/eyecu");
//    else
//        installer.setValue("TargetDir", "@HomeDir@/Road Works Software/eyecu");
        
    if (installer.addWizardPage(component, "InstallationTypePage", QInstaller.TargetDirectory)) {
        var widget = gui.pageWidgetByObjectName("DynamicInstallationTypePage");
        if (widget != null) {
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
            widget.windowTitle = "Select Installation Type";
            widget.startMenuCheckBox.checked=true;
        }
    }
}

Component.prototype.desktopToggled = function (checked) {
    component.setValue("Desktop", checked);
}

Component.prototype.startMenuToggled = function (checked) {
    installer.setSharedFlag("StartMenu", checked);
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
    component.setValue("Portable", checked);
}

Component.prototype.installAllUsersToggled = function (checked) {
    QMessageBox.information("test", "installAllUsersToggled", checked, QMessageBox.Ok);
    if (checked) {        
//        if (ComponentSelectionPage != null)
//            ComponentSelectionPage.selectDefault();
//        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
        installer.setValue("TargetDir", "@ApplicationsDir@/Road Works Software/eyecu");
        if (installer.value("os") === "win")
           installer.setValue("StartMenuDir", "@AllUsersStartMenuProgramsPath@/eyeCU");
    }
}

Component.prototype.installMeOnlyToggled = function (checked) {
    QMessageBox.information("test", "installMeOnlyToggled", checked, QMessageBox.Ok);
    if (checked) {
//        if (ComponentSelectionPage != null)
//            ComponentSelectionPage.selectAll();
//        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
        installer.setValue("TargetDir", "@HomeDir@/Road Works Software/eyecu");
        if (installer.value("os") === "win")
           installer.setValue("StartMenuDir", "@UserStartMenuProgramsPath@/eyeCU");
    }
}