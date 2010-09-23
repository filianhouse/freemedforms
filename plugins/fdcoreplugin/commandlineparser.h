/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2010 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <coreplugin/icommandline.h>

#include <QString>
#include <QVariant>



namespace Core {
namespace Internal {
class CommandLinePrivate;
}
class Patient;

class CommandLine  : public Core::ICommandLine
{
public:
    enum Param {
        CL_MedinTux = 0,
        CL_EMR_Name,
        CL_EMR_Uid,
        CL_SelectionOnly,
        CL_DrugsDatabaseUid,
        CL_ExchangeFile,
        CL_ExchangeFileFormat,
        CL_ExchangeInFile,
        CL_PatientName,
        CL_PatientFirstname,
        CL_PatientUid,
        CL_PatientGender,
        CL_DateOfBirth,
        CL_Weight,
        CL_Weight_Unit,
        CL_Height,
        CL_Height_Unit,
        CL_CrCl,
        CL_CrCl_Unit,
        CL_Creatinine,
        CL_Creatinine_Unit,
        CL_DrugsAllergies,
        CL_InnAllergies,
        CL_AtcAllergies,
        CL_DrugsIntolerances,
        CL_InnIntolerances,
        CL_AtcIntolerances,
        CL_ICD10Diseases,
        CL_Chrono,
        CL_TransmitDosage,
        CL_ConfigFile,
        CL_RunningUnderWine,
        CL_BlockPatientDatas,
        CL_MaxParam
    };

    CommandLine();
    ~ CommandLine();

    QVariant value(int param, const QVariant &def = QVariant()) const;
    QString paramName(int param) const;

    void feedPatientDatas(Core::Patient *patient);

private:
    Internal::CommandLinePrivate *d;
};

}

#endif // COMMANDLINEPARSER_H
