/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2018  Mike Tzou
 * Copyright (C) 2006  Christophe Dumez <chris@qbittorrent.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "foreignapps.h"

#include <QCoreApplication>
#include <QProcess>
#include <QStringList>

#include "base/logger.h"

/**
 * Detects the python version.
 */
int Utils::ForeignApps::Python::pythonVersion()
{
    static int version = -1;
    if (version < 0) {
        QString versionComplete = pythonVersionComplete().trimmed();
        QStringList splitted = versionComplete.split('.');
        if (splitted.size() > 1) {
            int highVer = splitted.at(0).toInt();
            if ((highVer == 2) || (highVer == 3))
                version = highVer;
        }
    }
    return version;
}

/**
 * Detects the python executable by calling "python --version".
 */
QString Utils::ForeignApps::Python::pythonExecutable()
{
    static QString executable;
    if (executable.isEmpty()) {
        QProcess pythonProc;
#if defined(Q_OS_UNIX)
        /*
         * On Unix-Like Systems python2 and python3 should always exist
         * http://legacy.python.org/dev/peps/pep-0394/
         */
        pythonProc.start("python3", {"--version"}, QIODevice::ReadOnly);
        if (pythonProc.waitForFinished() && (pythonProc.exitCode() == 0)) {
            executable = "python3";
            return executable;
        }
        pythonProc.start("python2", {"--version"}, QIODevice::ReadOnly);
        if (pythonProc.waitForFinished() && (pythonProc.exitCode() == 0)) {
            executable = "python2";
            return executable;
        }
#endif
        // Look for "python" in Windows and in UNIX if "python2" and "python3" are
        // not detected.
        pythonProc.start("python", {"--version"}, QIODevice::ReadOnly);
        if (pythonProc.waitForFinished() && (pythonProc.exitCode() == 0))
            executable = "python";
        else
            Logger::instance()->addMessage(QCoreApplication::translate("misc", "Python not detected"), Log::INFO);
    }
    return executable;
}

/**
 * Returns the complete python version
 * eg 2.7.9
 * Make sure to have setup python first
 */
QString Utils::ForeignApps::Python::pythonVersionComplete()
{
    static QString version;
    if (version.isEmpty()) {
        if (pythonExecutable().isEmpty())
            return version;
        QProcess pythonProc;
        pythonProc.start(pythonExecutable(), {"--version"}, QIODevice::ReadOnly);
        if (pythonProc.waitForFinished() && (pythonProc.exitCode() == 0)) {
            QByteArray output = pythonProc.readAllStandardOutput();
            if (output.isEmpty())
                output = pythonProc.readAllStandardError();

            // Software 'Anaconda' installs its own python interpreter
            // and `python --version` returns a string like this:
            // `Python 3.4.3 :: Anaconda 2.3.0 (64-bit)`
            const QList<QByteArray> outSplit = output.split(' ');
            if (outSplit.size() > 1) {
                version = outSplit.at(1).trimmed();
                Logger::instance()->addMessage(QCoreApplication::translate("misc", "Python version: %1").arg(version), Log::INFO);
            }

            // If python doesn't report a 3-piece version e.g. 3.6.1
            // then fill the missing pieces with zero
            const QStringList verSplit = version.split('.', QString::SkipEmptyParts);
            if (verSplit.size() < 3) {
                for (int i = verSplit.size(); i < 3; ++i) {
                    if (version.endsWith('.'))
                        version.append('0');
                    else
                        version.append(".0");
                }
                Logger::instance()->addMessage(QCoreApplication::translate("misc", "Normalized Python version: %1").arg(version), Log::INFO);
            }
        }
    }
    return version;
}
