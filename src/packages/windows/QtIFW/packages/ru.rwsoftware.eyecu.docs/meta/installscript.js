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
    installer.installationFinished.connect(this, Component.prototype.installationFinishedPageIsShown);
    installer.finishButtonClicked.connect(this, Component.prototype.installationFinished);
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install documentation!
    component.createOperations();
    if ((installer.value("os") == "win") && (installer.value("StartMenu") == "true")) {
		targetDocMenuDir = installer.value("StartMenuDir")+"\\Documentation";
		if (installer.value("AllUsers") == "true") {
//			targetDocMenuDir = targetDocMenuDir.replace(installer.value("UserStartMenuProgramsPath"), installer.value("AllUsersStartMenuProgramsPath"));
			component.addElevatedOperation("Mkdir", targetDocMenuDir);
			component.addElevatedOperation("CreateShortcut", "@TargetDir@/README.TXT", targetDocMenuDir+"\\README.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addElevatedOperation("CreateShortcut", "@TargetDir@/AUTHORS.TXT", targetDocMenuDir+"\\AUTHORS.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addElevatedOperation("CreateShortcut", "@TargetDir@/TRANSLATORS.TXT", targetDocMenuDir+"\\TRANSLATORS.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addElevatedOperation("CreateShortcut", "@TargetDir@/CHANGELOG.TXT", targetDocMenuDir+"\\CHANGELOG.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addElevatedOperation("CreateShortcut", "@TargetDir@/Licenses/LICENSE.TXT", targetDocMenuDir+"\\COPYING.lnk", "workingDirectory=@TargetDir@/Licenses", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
		} else {
			component.addOperation("Mkdir", targetDocMenuDir);
			component.addOperation("CreateShortcut", "@TargetDir@/README.TXT", targetDocMenuDir+"\\README.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addOperation("CreateShortcut", "@TargetDir@/AUTHORS.TXT", targetDocMenuDir+"\\AUTHORS.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addOperation("CreateShortcut", "@TargetDir@/TRANSLATORS.TXT", targetDocMenuDir+"\\TRANSLATORS.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addOperation("CreateShortcut", "@TargetDir@/CHANGELOG.TXT", targetDocMenuDir+"\\CHANGELOG.lnk", "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
			component.addOperation("CreateShortcut", "@TargetDir@/Licenses/LICENSE.TXT", targetDocMenuDir+"\\COPYING.lnk", "workingDirectory=@TargetDir@/Licenses", "iconPath=%SystemRoot%/system32/SHELL32.dll", "iconId=1");
		}
    }
}

Component.prototype.installationFinishedPageIsShown = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success)
            installer.addWizardPageItem( component, "ReadMeCheckBoxForm", QInstaller.InstallationFinished);
    } catch(e) {
        console.log(e);
    }
}

Component.prototype.installationFinished = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            var isReadMeCheckBoxChecked = component.userInterface( "ReadMeCheckBoxForm" ).readMeCheckBox.checked;
            if (isReadMeCheckBoxChecked)
                QDesktopServices.openUrl("file:///" + installer.value("TargetDir") + "/README.txt");
        }
    } catch(e) {
        console.log(e);
    }
}
