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

function Controller() {
	QMessageBox.information("controller.info", "Controller()", "begin!", QMessageBox.Ok);
	installer.uninstallationStarted.connect(onUninstallationStarted);
	QMessageBox.information("controller.info", "Controller()", "A!", QMessageBox.Ok);
	installer.updateFinished.connect(onUpdateFinished);
	QMessageBox.information("controller.info", "Controller()", "B!", QMessageBox.Ok);
	installer.installationFinished.connect(onUpdateFinished);
	QMessageBox.information("controller.info", "Controller()", "end!", QMessageBox.Ok);
}

onUninstallationStarted = function() {
	var targetPath = installer.value("TargetDir") + "\\eyecu.exe";
//	targetPath = targetPath[0].toLowerCase() + targetPath.substr(1);
	QMessageBox.information("controller.info", targetPath + " is running", installer.isProcessRunning(targetPath), QMessageBox.Ok);
	if (installer.killProcess("eyecu.exe"))
		QMessageBox.information("controller.info", targetPath, "Killed", QMessageBox.Ok);
	else
		QMessageBox.error("controller.info", targetPath, "NOT Killed", QMessageBox.Ok);
	
}

onUpdateFinished = function() {
	QMessageBox.information("controller.info", "Controller()", "onUpdateFinished", QMessageBox.Ok);
	var targetPath = installer.value("TargetDir") + "\\eyecu.exe";
//	targetPath = targetPath[0].toLowerCase() + targetPath.substr(1);
	QMessageBox.information("controller.info", targetPath + " is running", installer.isProcessRunning(targetPath), QMessageBox.Ok);
	if (installer.isProcessRunning(targetPath)) {
		QMessageBox.information("controller.info", "onUpdateFinished", "A", QMessageBox.Ok);
		var page = gui.pageById(QInstaller.InstallationFinished);
		if (page != null) {
			QMessageBox.information("controller.info", "onUpdateFinished", "A", QMessageBox.Ok);
			page.RunItCheckBox.checked = false;
			page.RunItCheckBox.enabled = false;
		}
	}

}