/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developpers :                                                    *
 *       Eric MAEKER, MD <eric.maeker@gmail.com>                           *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "pack.h"
#include "datapackcore.h"

#include <utils/log.h>
#include <utils/global.h>

#include <QDomDocument>
#include <QDomElement>
#include <QDir>

using namespace DataPack;

static inline DataPack::DataPackCore &core() {return DataPack::DataPackCore::instance();}

namespace {
const char *const TAG_ROOT = "DataPack_Pack";
const char *const TAG_PACKDESCRIPTION = "PackDescription";
const char *const TAG_PACKDEPENDENCIES = "PackDependencies";
}

Pack::Pack() :
    m_Sha1Checked(false), m_Md5Checked(false)
{
}

Pack::~Pack()
{
}

QString Pack::uuid() const
{
    return m_descr.data(PackDescription::Uuid).toString();
}

QString Pack::version() const
{
    return m_descr.data(PackDescription::Version).toString();
}

QString Pack::name() const
{
    return m_descr.data(PackDescription::Label).toString();
}

QString Pack::serverFileName() const
{
    return m_descr.data(PackDescription::AbsFileName).toString();
}

QString Pack::serverLicenceFileName() const
{
//    return m_descr.data(PackDescription::LicenceFileName).toString();
    return QString();
}

QString Pack::md5ControlChecksum() const
{
    return m_descr.data(PackDescription::Md5).toString();
}

QString Pack::sha1ControlChecksum() const
{
    return m_descr.data(PackDescription::Sha1).toString();
}

bool Pack::isSha1Checked() const
{
    // TODO
    // Une fois le fichier télécharger, vérifie la sha1 locale avec celle donnée par le serveur
}

bool Pack::isMd5Checked() const
{
    // TODO
    // Une fois le fichier télécharger, vérifie la md5 locale avec celle donnée par le serveur

}

/** Return the original file name of the pack XML config file. This file name is only valid on local servers. */
QString Pack::originalXmlConfigFileName() const
{
    return m_OriginalFileName;
}

/** Return the persistentCached file name of the pack XML config file. This file name is computed using the DataPack::DataPackCore::persistentCachePath(). */
QString Pack::persistentlyCachedXmlConfigFileName() const
{
    return core().persistentCachePath() + QDir::separator() + uuid() + QDir::separator() + "packconfig.xml";
}

/** Return the persistentCached file name of the zipped pack file. This file name is computed using the DataPack::DataPackCore::persistentCachePath(). */
QString Pack::persistentlyCachedZipFileName() const
{
    return core().persistentCachePath() + QDir::separator() + uuid() + QDir::separator() + QFileInfo(serverFileName()).fileName();
}

/** Return the path where to unzip the pack zipped file. This path is computed using the DataPack::DataPackCore::installPath(). */
QString Pack::unzipPackToPath() const
{
    return core().installPath() + QDir::separator() + m_descr.data(PackDescription::UnzipToPath).toString();
}

/** Return the installed file name of the pack XML config file. This file name is computed using the DataPack::DataPackCore::installPath(). */
QString Pack::installedXmlConfigFileName() const
{
    return unzipPackToPath() + QDir::separator() + "packconfig.xml";
}

/** Return the installed file name of the zipped pack file. This file name is computed using the DataPack::DataPackCore::installPath(). */
QString Pack::installedZipFileName() const
{
    /** \todo code here : missing extracted zip file name. */
    return unzipPackToPath();
}

/**
 * Reads the XML configuration file of the pack and
 * create the DataPack::PackDescription and the DataPack::PackDependencies
 * related to this pack.
*/
void Pack::fromXmlFile(const QString &absFileName)
{
    m_OriginalFileName = absFileName;
    fromXml(Utils::readTextFile(absFileName, Utils::DontWarnUser));
}

/**
 * Reads the XML configuration content of the pack and
 * create the DataPack::PackDescription and the DataPack::PackDependencies
 * related to this pack.
*/
void Pack::fromXml(const QString &fullPackConfigXml)
{
    QDomDocument doc;
    if (!doc.setContent(fullPackConfigXml)) {
        LOG_ERROR_FOR("DataPack::Pack", "Wrong XML");
        return;
    }
    QDomElement root = doc.firstChildElement(::TAG_ROOT);
    QDomElement descr = root.firstChildElement(::TAG_PACKDESCRIPTION);
    QDomElement dep = root.firstChildElement(::TAG_PACKDEPENDENCIES);
    m_descr.fromDomElement(descr);
    m_depends.fromDomElement(dep);
}

bool Pack::operator==(const Pack &other) const
{
    return (this->uuid()==other.uuid() &&
            this->md5ControlChecksum()==other.md5ControlChecksum() &&
            this->sha1ControlChecksum()==other.sha1ControlChecksum());
}

QDebug operator<<(QDebug dbg, const DataPack::Pack &p)
{
    dbg.nospace() << "Pack("
                  << p.uuid()
                  << ", "
                  << p.version()
                  << ", "
                  << p.serverFileName()
                  << ")";
    return dbg.space();
}


