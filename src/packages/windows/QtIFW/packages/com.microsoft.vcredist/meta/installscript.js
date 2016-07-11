/**************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
**
** $QT_END_LICENSE$
**
**************************************************************************/

function Component()
{
}

Component.prototype.createOperationsForArchive = function(archive)
{
	temp = QDesktopServices.storageLocation(QDesktopServices.TempLocation)

	if (temp != "")
	{
		tmpdir = temp+"/eyecu-msvcrt-redist/";
		component.addOperation("Extract", archive, tmpdir);
		component.addOperation("Execute","{0,1612,1618,3010}", tmpdir+"vcredist_x86.exe", "/q");
		component.addOperation("Delete", tmpdir+"vcredist_x86.exe");
//		component.addOperation("Delete", tmpdir+"vcredist_x86.exe.*");
//		component.addOperation("Rmdir", tmpdir);
	}
	else
	{
		QMessageBox.error("component.error", "Error!", "Failed to extract Microsoft Visual C Runtime installer.\nNeither TMP nor TEMP environment variable set!", QMessageBox.Ok);
		installer.interrupt();
	}
}
