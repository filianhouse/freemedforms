/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef DDIMANAGER_DDIPLUGIN_COMPONENTLINKERDATA_H
#define DDIMANAGER_DDIPLUGIN_COMPONENTLINKERDATA_H

#include <ddiplugin/ddi_exporter.h>

#include <QStringList>
#include <QHash>
#include <QMultiHash>

/**
 * \file componentlinkerdata.h
 * \author Eric Maeker
 * \version 0.10.0
 * \date 28 Jan 2014
*/

namespace DrugsDB {
namespace Internal {
class DrugBaseEssentials;
}
}

namespace DDI {
class ComponentAtcModel;

class DDI_EXPORT ComponentLinkerData
{
    friend class DDI::ComponentAtcModel;

public:
    /**
     * DDI::ComponentAtcModel needs data to link components with interactors.
     * This class is a container for these data.
     * \sa DDI::ComponentAtcModel::startComponentLinkage()
     */
    ComponentLinkerData() :
        lang("fr")
    {}

    ~ComponentLinkerData()
    {}

    // In setData
    /**
     * Define the language to use for Component to ATC label matching (eg: fr, en, de, sp). \n
     * This data must be defined before the DDI::ComponentModel will try to link drugs
     * components to drug interactors / ATC codes. \n
     * By default, the language is defined to \e french.
    */
    void setAtcLanguage(const QString &_lang) {lang = lang;}

    /**
     * Define some hand made correction for Component using ATC \e label.
     * Key is the component name, value is the ATC \e label to link with. \n
     * All names and codes must be uppercase. \n
     * This data must be defined before the DDI::ComponentModel will try to
     * link drugs components to drug interactors / ATC codes.
     */
    void setComponentCorrectionByName(const QHash<QString, QString> &correction) {correctedByName = correction;}

    /**
     * Define some hand made correction for Component using ATC \e code.
     * Key is the component name, value is the ATC \e code to link with.
     * All names and codes must be uppercase. \n
     * This data must be defined before the DDI::ComponentModel will try to
     * link drugs components to drug interactors / ATC codes.
     */
    void setComponentCorrectionByAtcCode(const QMultiHash<QString, QString> &correction) {correctedByAtcCode = correction;}

    /**
     * You must define the database identifiants for the \e drugs \e components. \n
     * Key is the \e name of the component, value its database Id. All names and
     * codes must be uppercase. \n
     * This data must be defined before the DDI::ComponentModel will try to
     * link drugs components to drug interactors / ATC codes.
     */
    void setComponentIds(const QHash<QString, int> &_compoIds) {compoIds = _compoIds;}

    /**
     * You must define the database identifiants for the \e ATC \e code. \n
     * This data must be defined before the DDI::ComponentModel will try to
     * link drugs components to drug interactors / ATC codes.
     */
    void setAtcCodeIds(const QHash<QString, int> &_atcCodeIds) {atcCodeIds = _atcCodeIds;}


protected:
    // In data
    QString lang;
    QHash<QString, QString> correctedByName;
    QMultiHash<QString, QString> correctedByAtcCode;
    QHash<QString, int> compoIds;
    QHash<QString, int> atcCodeIds;
};

class DDI_EXPORT ComponentLinkerResult
{
    friend class DDI::ComponentAtcModel;

public:
    /**
     * DDI::ComponentAtcModel manages components with interactors linking.
     * This class is a container for the processing results.
     * \sa DDI::ComponentAtcModel::startComponentLinkage()
     */
    ComponentLinkerResult() :
        completionPercentage(0.0)
    {}

    ~ComponentLinkerResult()
    {}

    /** Returns the list of error messages */
    const QStringList &errors() const {return _errors;}

    /** Returns the list of processing messages */
    const QStringList &messages() const {return _msg;}

    /** Returns the component id to ATC id links */
    const QMultiHash<int, int> &componentIdToAtcId() const {return compoIdToAtcId;}

    /**
     * Returns true if the result already contains linking data for the specific
     * drug component ID.
     */
    bool containsComponentId(const int componentId) const {return compoIdToAtcId.uniqueKeys().contains(componentId);}

protected:
    // Out setData
    /**
     * Define the component name list that can not be linked to any drug interactor / ATC code. \n
     * This data is defined by the DDI::ComponentModel::componentLinker() procedure.
     */
    void setUnfoundComponents(const QStringList &unfound) {unfoundComponentsAssociation = unfound;}

    /**
     * Define the component name list that can not be linked to any drug interactor / ATC code. \n
     * This data is defined by the DDI::ComponentModel::componentLinker() procedure.
     */
    void addUnfoundComponent(const QString &unfound)
    {
        if (!unfoundComponentsAssociation.contains(unfound))
            unfoundComponentsAssociation << unfound;
    }

    /**
     * Define the link between the component database Id and the ATC database Id.
     * This data is defined by the DDI::ComponentModel::componentLinker() procedure.
     */
    void setComponentsIdToAtcId(const QMultiHash<int, int> &links) {compoIdToAtcId = links;}

    /** Add an error message */
    void addErrorMessage(const QString &s) {_errors << s;}

    /** Add a message */
    void addMessage(const QString &s) {_msg << s;}

    /** Add a link between component and interactor */
    void addComponentToAtcLink(int componentId, int atcId) {compoIdToAtcId.insertMulti(componentId, atcId);}

protected:
    QStringList unfoundComponentsAssociation, _errors, _msg;
    double completionPercentage;
    QMultiHash<int, int> compoIdToAtcId; // Key: moleculeId, Values: AtcIds
};

} // namespace DDI

#endif // DDIMANAGER_DDIPLUGIN_COMPONENTLINKERDATA_H
