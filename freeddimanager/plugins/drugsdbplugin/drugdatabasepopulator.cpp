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
 *  Main developer: Eric MAEKER, <eric.maeker@gmail.com>                   *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
/*!
 * \class DrugsDb::Internal::DatabasePopulator
 * Populate a Drug database with DDI information (ATC codes, DDI, PIMs and
 * all other interaction engines data).\n
 */

#include "drugdatabasepopulator.h"
#include <drugsdbplugin/tools.h>

#include <ddiplugin/ddi/drugdruginteraction.h>
#include <ddiplugin/interactors/druginteractor.h>
#include <ddiplugin/ddicore.h>
#include <ddiplugin/database/ddidatabase.h>
#include <ddiplugin/constants.h>

#include <drugsbaseplugin/drugbaseessentials.h>
#include <drugsbaseplugin/constants_databaseschema.h>

//#include <biblio/bibliocore.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/fdm_constants.h>

#include <utils/log.h>
#include <translationutils/constants.h>
#include <translationutils/trans_msgerror.h>
#include <translationutils/trans_database.h>

#include <QHash>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>

#include <QDebug>

using namespace DDI;
using namespace DrugsDb;
using namespace DrugsDb::Internal;
using namespace Trans::ConstantTranslations;

static inline DDI::DDICore *ddiCore() {return DDI::DDICore::instance();}
static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }

namespace  {
const int CLASS_OR_MOL_ID = 65000;
const int FREEMEDFORMS_ATC_CODE = 65001;

static inline bool connectDatabase(QSqlDatabase &DB, const QString &file, const int line)
{
    if (!DB.isOpen()) {
        if (!DB.open()) {
            Utils::Log::addError("DrugDatabasePopulator", tkTr(Trans::Constants::UNABLE_TO_OPEN_DATABASE_1_ERROR_2)
                                 .arg(DB.connectionName()).arg(DB.lastError().text()),
                                 file, line);
            return false;
        }
    }
    return true;
}
}

namespace DrugsDb {
namespace Internal {
class DrugDatabasePopulatorPrivate
{
public:
    DrugDatabasePopulatorPrivate(DrugDatabasePopulator *parent) :
        q(parent)
    {
    }

    ~DrugDatabasePopulatorPrivate()
    {
    }

    /** Save ATC data: class interactors */
    bool saveClassDrugInteractor(DrugInteractor *interactor, const QList<DrugInteractor *> &completeList, DrugsDB::Internal::DrugBaseEssentials *database, DrugInteractor *parent)
    {
        QSqlDatabase db = database->database();
        if (!connectDatabase(db, __FILE__, __LINE__))
            return false;

        // save interactor
        QSqlQuery query(db);
        int id = -1;
        // save using all associated ATC codes
        const QStringList &atcCodes = interactor->data(DrugInteractor::ATCCodeStringList).toStringList();
        if (atcCodes.isEmpty() && !interactor->isClass() && parent && parent->isClass()) {
            //        QString req = QString("INSERT INTO ATC_CLASS_TREE (ID_TREE, ID_CLASS, ID_ATC) VALUES "
            //                              "(NULL, %1,%2);")
            //                .arg(parent->data(CLASS_OR_MOL_ID).toString())
            //                .arg(interactor->data(CLASS_OR_MOL_ID).toString());
            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_ATC_CLASS_TREE));
            query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_ID, QVariant());
            query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_ID_ATC, interactor->data(CLASS_OR_MOL_ID).toString());
            query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_ID_CLASS, parent->data(CLASS_OR_MOL_ID).toString());
            query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_BIBMASTERID, QVariant());

            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR(q, query);
            } else {
                id = query.lastInsertId().toInt();
            }
            query.finish();
        } else if (!atcCodes.isEmpty() && !interactor->isClass() && parent && parent->isClass()) {
            foreach(const QString &atc, atcCodes) {
                //            QString req = QString("INSERT INTO ATC_CLASS_TREE (ID_TREE, ID_CLASS, ID_ATC) VALUES "
                //                                  "(NULL, %1, (SELECT ATC_ID FROM ATC WHERE CODE=\"%2\"));")
                //                    .arg(parent->data(CLASS_OR_MOL_ID).toString()).arg(atc);

                QString atcId;
                QHash<int, QString> w;
                w.insert(DrugsDB::Constants::ATC_CODE, QString("='%1'").arg(atc));
                QString req = database->select(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID, w);
                if (query.exec(req)) {
                    if (query.next())
                        atcId = query.value(0).toString();
                } else {
                    LOG_QUERY_ERROR_FOR(q, query);
                    db.rollback();
                    return false;
                }
                query.finish();

                query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_ATC_CLASS_TREE));
                query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_ID, QVariant());
                query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_ID_ATC, atcId);
                query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_ID_CLASS, parent->data(CLASS_OR_MOL_ID).toString());
                query.bindValue(DrugsDB::Constants::ATC_CLASS_TREE_BIBMASTERID, QVariant());

                if (!query.exec()) {
                    LOG_QUERY_ERROR_FOR(q, query);
                } else {
                    id = query.lastInsertId().toInt();
                }
                query.finish();
            }
        }

        // add pmids references
        if (id>=0) {
            foreach(const QString &pmid, parent->childClassificationPMIDs(interactor->data(DrugInteractor::InitialLabel).toString())) {
                m_iamTreePmids.insertMulti(id, pmid);
            }
        }

        // if class, include all its children recursively
        if (interactor->isClass()) {
            foreach(const QString &childId, interactor->childrenIds()) {
                // find pointer to the child
                DrugInteractor *child = 0;
                for(int j=0; j < completeList.count();++j) {
                    DrugInteractor *testMe = completeList.at(j);
                    if (testMe->data(DrugInteractor::InitialLabel).toString()==childId) {
                        child = testMe;
                        break;
                    }
                }
                if (child)
                    if (!saveClassDrugInteractor(child, completeList, database, interactor))
                        return false;
            }
        }
        return true;
    }

    /** Save the DDI data to the drug database */
    bool saveDrugDrugInteractions(DrugsDB::Internal::DrugBaseEssentials *database, const QList<DrugInteractor *> &interactors, const QList<DrugDrugInteraction *> &ddis)
    {
        QSqlDatabase db = database->database();
        if (!connectDatabase(db, __FILE__, __LINE__))
            return false;
        LOG_FOR(q, "Saving drug drug interactions");

        // Clear database first
        QString req;
//        req = database->prepareDeleteQuery(DrugsDB::Constants::Table_INTERACTIONS);
//        DrugsDB::Tools::executeSqlQuery(req, database->connectionName(), __FILE__, __LINE__);
//        req = database->prepareDeleteQuery(DrugsDB::Constants::Table_IAKNOWLEDGE);
//        DrugsDB::Tools::executeSqlQuery(req, database->connectionName(), __FILE__, __LINE__);
//        req = database->prepareDeleteQuery(DrugsDB::Constants::Table_IA_IAK);
//        DrugsDB::Tools::executeSqlQuery(req, database->connectionName(), __FILE__, __LINE__);

        db.transaction();

        for(int i = 0; i < ddis.count(); ++i) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            DrugDrugInteraction *ddi = ddis.at(i);

            // find first && second interactors
            bool firstFound = false;
            bool secondFound = false;
            const QString &first = ddi->firstInteractor();
            const QString &second = ddi->secondInteractor();
            DrugInteractor *firstInteractor = 0;
            DrugInteractor *secondInteractor = 0;
            for(int i=0; i < interactors.count();++i) {
                const QString &id = interactors.at(i)->data(DrugInteractor::InitialLabel).toString();
                if (!firstFound) {
                    if (id==first) {
                        firstFound = true;
                        firstInteractor = interactors.at(i);
                    }
                }
                if (!secondFound) {
                    if (id==second) {
                        secondFound = true;
                        secondInteractor = interactors.at(i);
                    }
                }
                if (firstFound && secondFound)
                    break;
            }
            bool ok = (firstFound && secondFound);
            if (!ok) {
                LOG_ERROR_FOR(q, QString("*** Interactors not found: \n  %1 - %2 (%3)")
                          .arg(ddi->firstInteractor())
                          .arg(ddi->secondInteractor())
                          .arg(ddi->levelName()));
                continue;
            }

            QSqlQuery query(db);
            QString req;
            QList<int> ia_ids;
            int iak_id = -1;

            // for all atc of firstInteractor & all atc of secondInteractor -> add an interaction + keep the references of the DDI
            QStringList atc1 = firstInteractor->data(DrugInteractor::ATCCodeStringList).toStringList();
            QStringList atc2 = secondInteractor->data(DrugInteractor::ATCCodeStringList).toStringList();
            atc1.removeAll("");
            atc1.removeDuplicates();
            atc2.removeAll("");
            atc2.removeDuplicates();
            if (atc1.isEmpty())
                atc1 << firstInteractor->data(FREEMEDFORMS_ATC_CODE).toString();
            if (atc2.isEmpty())
                atc2 << secondInteractor->data(FREEMEDFORMS_ATC_CODE).toString();
            foreach(const QString &a1, atc1) {
                foreach(const QString &a2, atc2) {
                    QString atcId1;
                    QHash<int, QString> w;
                    w.insert(DrugsDB::Constants::ATC_CODE, QString("='%1'").arg(a1));
                    req = database->select(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID, w);
                    if (query.exec(req)) {
                        if (query.next())
                            atcId1 = query.value(0).toString();
                    } else {
                        LOG_QUERY_ERROR_FOR(q, query);
                        db.rollback();
                        return false;
                    }
                    query.finish();

                    QString atcId2;
                    w.clear();
                    w.insert(DrugsDB::Constants::ATC_CODE, QString("='%1'").arg(a2));
                    req = database->select(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID, w);
                    if (query.exec(req)) {
                        if (query.next())
                            atcId2 = query.value(0).toString();
                    } else {
                        LOG_QUERY_ERROR_FOR(q, query);
                        db.rollback();
                        return false;
                    }
                    query.finish();

                    //                req = QString("INSERT INTO INTERACTIONS (ATC_ID1, ATC_ID2) VALUES (%1, %2);")
                    //                        .arg(QString("(SELECT ATC_ID FROM ATC WHERE CODE=\"%1\")").arg(a1))
                    //                        .arg(QString("(SELECT ATC_ID FROM ATC WHERE CODE=\"%1\")").arg(a2));
                    query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_INTERACTIONS));
                    query.bindValue(DrugsDB::Constants::INTERACTIONS_IAID, QVariant());
                    query.bindValue(DrugsDB::Constants::INTERACTIONS_ATC_ID1, atcId1);
                    query.bindValue(DrugsDB::Constants::INTERACTIONS_ATC_ID2, atcId2);
                    if (!query.exec()) {
                        LOG_QUERY_ERROR_FOR(q, query);
                        LOG_ERROR_FOR(q, QString("*** Interactors not found: \n  %1 - %2 (%3)")
                                  .arg(ddi->data(DrugDrugInteraction::FirstInteractorName).toString())
                                  .arg(ddi->data(DrugDrugInteraction::SecondInteractorName).toString())
                                  .arg(ddi->data(DrugDrugInteraction::LevelName).toString()));
                        db.rollback();
                        return false;
                    } else {
                        ia_ids << query.lastInsertId().toInt();
                    }
                    query.finish();

                    // mirror DDI
                    //                req = QString("INSERT INTO INTERACTIONS (ATC_ID2, ATC_ID1) VALUES (%1, %2);")
                    //                        .arg(QString("(SELECT ATC_ID FROM ATC WHERE CODE=\"%1\")").arg(a1))
                    //                        .arg(QString("(SELECT ATC_ID FROM ATC WHERE CODE=\"%1\")").arg(a2));
                    query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_INTERACTIONS));
                    query.bindValue(DrugsDB::Constants::INTERACTIONS_IAID, QVariant());
                    query.bindValue(DrugsDB::Constants::INTERACTIONS_ATC_ID1, atcId2);
                    query.bindValue(DrugsDB::Constants::INTERACTIONS_ATC_ID2, atcId1);
                    if (!query.exec()) {
                        LOG_QUERY_ERROR_FOR(q, query);
                        LOG_ERROR_FOR(q, QString("*** Interactors not found: \n  %1 - %2 (%3)")
                                  .arg(ddi->data(DrugDrugInteraction::FirstInteractorName).toString())
                                  .arg(ddi->data(DrugDrugInteraction::SecondInteractorName).toString())
                                  .arg(ddi->data(DrugDrugInteraction::LevelName).toString()));
                        db.rollback();
                        return false;
                    } else {
                        ia_ids << query.lastInsertId().toInt();
                    }
                    query.finish();
                }
            }

            // Add labels
            QMultiHash<QString, QVariant> risk;
            risk.insert("fr", ddi->risk("fr"));
            risk.insert("en", ddi->risk("en"));
            QMultiHash<QString, QVariant> management;
            management.insert("fr", ddi->management("fr"));
            management.insert("en", ddi->management("en"));
            int riskMasterLid = DrugsDb::Tools::addLabels(database, -1, risk);
            int manMasterLid = DrugsDb::Tools::addLabels(database, -1, management);
            if (riskMasterLid==-1 || manMasterLid==-1)
                return false;

            // Add IAK
            //        req = QString("INSERT INTO IAKNOWLEDGE (IAKID, TYPE, RISK_MASTER_LID, MAN_MASTER_LID) VALUES "
            //                      "(NULL, \"%1\", %2, %3)")
            //                .arg(ddi->data(DrugDrugInteraction::LevelCode).toString())
            //                .arg(riskMasterLid)
            //                .arg(manMasterLid);

            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_IAKNOWLEDGE));
            query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_IAKID, QVariant());
            query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_TYPE, ddi->data(DrugDrugInteraction::LevelCode).toString());
            query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_RISK_MASTERLID, riskMasterLid);
            query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_BIB_MASTERID, QVariant());
            query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_MANAGEMENT_MASTERLID, manMasterLid);
            query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_WWW, QVariant());
            if (query.exec()) {
                // keep trace of bibliographic references
                iak_id = query.lastInsertId().toInt();
                foreach(const QString &pmid, ddi->data(DrugDrugInteraction::PMIDsStringList).toStringList())
                    m_ddiPmids.insertMulti(iak_id, pmid);
            } else {
                LOG_QUERY_ERROR_FOR(q, query);
                db.rollback();
                return false;
            }
            query.finish();

            // Add to IA_IAK link table
            foreach(const int ia, ia_ids) {
                //            req = QString("INSERT INTO IA_IAK (IAID, IAKID) VALUES (%1,%2)")
                //                    .arg(ia)
                //                    .arg(iak_id)
                //                    ;
                query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_IA_IAK));
                query.bindValue(DrugsDB::Constants::IA_IAK_IAID, ia);
                query.bindValue(DrugsDB::Constants::IA_IAK_IAKID, iak_id);
                if (!query.exec()) {
                    LOG_QUERY_ERROR_FOR(q, query);
                    db.rollback();
                    return false;
                }
                query.finish();
            }
        }

        db.commit();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        LOG_FOR(q, "Drug drug interactions saved");

        return true;
    }

    /** Save all needed bibliographic references to a drug database */
    bool saveBibliographicReferences(DrugsDB::Internal::DrugBaseEssentials *database)
    {
        LOG_FOR(q, "Saving bibliographic references in: " + database->database().databaseName());
        // TODO: Ensure all PMIDs are available */
        QSqlDatabase db = database->database();
        if (!connectDatabase(db, __FILE__, __LINE__))
            return false;

        // Clear database first
//        QString req = database->prepareDeleteQuery(DrugsDB::Constants::Table_BIB);
//        DrugsDb::Tools::executeSqlQuery(req, database->connectionName(), __FILE__, __LINE__);
//        req = database->prepareDeleteQuery(DrugsDB::Constants::Table_BIB_LINK);
//        DrugsDb::Tools::executeSqlQuery(req, database->connectionName(), __FILE__, __LINE__);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        // Save all pmids
        QHash<QString, int> pmidsBibliographyId;
        QStringList pmids;
        pmids << m_iamTreePmids.values();
        pmids << m_ddiPmids.values();
        pmids.removeAll("");
        pmids.removeDuplicates();
        QSqlQuery query(db);
        foreach(const QString &pmid, pmids) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_BIB));
            query.bindValue(DrugsDB::Constants::BIB_BIBID, QVariant());
            query.bindValue(DrugsDB::Constants::BIB_TYPE, "pubmed");
            query.bindValue(DrugsDB::Constants::BIB_LINK, QString("http://www.ncbi.nlm.nih.gov/pubmed/%1?dopt=Abstract&format=text").arg(pmid));
            query.bindValue(DrugsDB::Constants::BIB_TEXTREF, QVariant());
            query.bindValue(DrugsDB::Constants::BIB_ABSTRACT, QVariant());
            query.bindValue(DrugsDB::Constants::BIB_EXPLAIN, QVariant());
//            query.bindValue(DrugsDB::Constants::BIB_XML, Biblio::BiblioCore::instance()->xml(pmid).replace("'","''"));
            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR(q, query);
                return false;
            } else {
                pmidsBibliographyId.insert(pmid, query.lastInsertId().toInt());
            }
            query.finish();
        }

        // Save all tree references
        int bibMasterId = 0;
        foreach(int key, m_iamTreePmids.uniqueKeys()) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            const QStringList &pmids = m_iamTreePmids.values(key);
            ++bibMasterId;
            QHash<int, QString> w;
            w.insert(DrugsDB::Constants::ATC_CLASS_TREE_ID, QString("='%1'").arg(key));
            query.prepare(database->prepareUpdateQuery(DrugsDB::Constants::Table_ATC_CLASS_TREE,
                                                       DrugsDB::Constants::ATC_CLASS_TREE_BIBMASTERID,
                                                       w));
            query.bindValue(0, bibMasterId);
            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR(q, query);
                return false;
            }
            query.finish();
            foreach(const QString &pmid, pmids) {
                // create the master_id for this pmid
                query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_BIB_LINK));
                query.bindValue(DrugsDB::Constants::BIB_LINK_BIBID, pmidsBibliographyId.value(pmid));
                query.bindValue(DrugsDB::Constants::BIB_LINK_MASTERID, bibMasterId);
                if (!query.exec()) {
                    LOG_QUERY_ERROR_FOR(q, query);
                    return false;
                }
                query.finish();
            }
        }

        // Save DDI references
        foreach(int key, m_ddiPmids.uniqueKeys()) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            const QStringList &pmids = m_ddiPmids.values(key);
            ++bibMasterId;
            QHash<int, QString> w;
            w.insert(DrugsDB::Constants::IAKNOWLEDGE_IAKID, QString("='%1'").arg(key));
            query.prepare(database->prepareUpdateQuery(DrugsDB::Constants::Table_IAKNOWLEDGE,
                                                       DrugsDB::Constants::IAKNOWLEDGE_BIB_MASTERID,
                                                       w));
            query.bindValue(0, bibMasterId);
            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR(q, query);
                return false;
            }
            query.finish();
            foreach(const QString &pmid, pmids) {
                // create the master_id for this pmid
                query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_BIB_LINK));
                query.bindValue(DrugsDB::Constants::BIB_LINK_BIBID, pmidsBibliographyId.value(pmid));
                query.bindValue(DrugsDB::Constants::BIB_LINK_MASTERID, bibMasterId);
                if (!query.exec()) {
                    LOG_QUERY_ERROR_FOR(q, query);
                    return false;
                }
                query.finish();
            }
        }

        LOG_FOR(q, "Bibliographic references saved");
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        return true;
    }

    // Read the FreeDDIManager DDIDatabase and return the list of valid DDI
    QList<DrugDrugInteraction *> getDrugDrugInteractions(DrugsDB::Internal::DrugBaseEssentials *database)
    {
        QSqlDatabase db = database->database();
        QList<DrugDrugInteraction *> list;
        if (!connectDatabase(db, __FILE__, __LINE__))
            return list;
        db.transaction();
        QSqlQuery query(db);
        QHash<int, QString> where;
        where.insert(Constants::DDI_ISVALID, "=1");
        QString req = ddiCore()->database().select(Constants::Table_DDI, where);
        if (query.exec(req)) {
            while (query.next()) {
                DrugDrugInteraction *ddi = new DrugDrugInteraction;
                ddi->setData(DrugDrugInteraction::FirstInteractorName, query.value(Constants::DDI_FIRSTINTERACTORUID));
                ddi->setData(DrugDrugInteraction::SecondInteractorName, query.value(Constants::DDI_SECONDINTERACTORUID));
                ddi->setData(DrugDrugInteraction::FirstInteractorRouteOfAdministrationIds, query.value(Constants::DDI_FIRSTINTERACTORROUTEOFADMINISTRATIONIDS));
                ddi->setData(DrugDrugInteraction::SecondInteractorRouteOfAdministrationIds, query.value(Constants::DDI_SECONDINTERACTORROUTEOFADMINISTRATIONIDS));
                ddi->setData(DrugDrugInteraction::LevelCode, query.value(Constants::DDI_LEVELCODE));
                ddi->setData(DrugDrugInteraction::DateCreation, query.value(Constants::DDI_DATECREATE));
                ddi->setData(DrugDrugInteraction::DateLastUpdate, query.value(Constants::DDI_DATEUPDATE));
                // ddi->setData(DrugDrugInteraction::ListOfUpdates, query.value(Constants::DDI_));
                // ddi->setData(DrugDrugInteraction::IsDuplicated, query.value());
                ddi->setData(DrugDrugInteraction::IsValid, query.value(Constants::DDI_ISVALID));
                ddi->setData(DrugDrugInteraction::IsReviewed, query.value(Constants::DDI_ISREVIEWED));
                ddi->setData(DrugDrugInteraction::ReviewersStringList, query.value(Constants::DDI_REVIEWERSSTRINGLIST));
                ddi->setData(DrugDrugInteraction::Comment, query.value(Constants::DDI_COMMENT));
                ddi->setData(DrugDrugInteraction::PMIDsStringList, query.value(Constants::DDI_PMIDSTRINGLIST));
                ddi->setData(DrugDrugInteraction::InternalUuid, query.value(Constants::DDI_UID));
                ddi->setData(DrugDrugInteraction::Source, query.value(Constants::DDI_SOURCE));
                ddi->setRisk(query.value(Constants::DDI_RISKFR).toString(), "fr");
                ddi->setRisk(query.value(Constants::DDI_RISKEN).toString(), "en");
                ddi->setManagement(query.value(Constants::DDI_MANAGEMENTFR).toString(), "fr");
                ddi->setManagement(query.value(Constants::DDI_MANAGEMENTEN).toString(), "en");
                // TODO: add management of dose
                // Constants::DDI_FIRSTDOSEUSEFROM,
                // Constants::DDI_FIRSTDOSEUSESTO,
                // Constants::DDI_FIRSTDOSEFROMVALUE,
                // Constants::DDI_FIRSTDOSEFROMUNITS,          // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::UNITS() STRINGLIST
                // Constants::DDI_FIRSTDOSEFROMREPARTITION,    // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::REPARTITION() STRINGLIST
                // Constants::DDI_FIRSTDOSETOVALUE,
                // Constants::DDI_FIRSTDOSETOUNITS,            // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::UNITS() STRINGLIST
                // Constants::DDI_FIRSTDOSETOREPARTITION,      // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::REPARTITION() STRINGLIST
                // Constants::DDI_SECONDDOSEUSEFROM,
                // Constants::DDI_SECONDDOSEUSESTO,
                // Constants::DDI_SECONDDOSEFROMVALUE,
                // Constants::DDI_SECONDDOSEFROMUNITS,         // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::UNITS() STRINGLIST
                // Constants::DDI_SECONDDOSEFROMREPARTITION,   // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::REPARTITION() STRINGLIST
                // Constants::DDI_SECONDDOSETOVALUE,
                // Constants::DDI_SECONDDOSETOUNITS,           // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::UNITS() STRINGLIST
                // Constants::DDI_SECONDDOSETOREPARTITION,     // RETURN THE ID IN THE DRUGDRUGINTERACTIONMODEL::REPARTITION() STRINGLIST
                list << ddi;
            }
        } else {
            LOG_QUERY_ERROR_FOR(q, query);
            query.finish();
            db.commit();
            return list;
        }
        query.finish();
        db.commit();
        return list;
    }

    // Read the FreeDDIManager DDIDatabase and return the list of valid DrugInteractors
    QList<DrugInteractor *> getDrugInteractors(DrugsDB::Internal::DrugBaseEssentials *database)
    {
        QSqlDatabase db = database->database();
        QList<DrugInteractor *> list;
        if (!connectDatabase(db, __FILE__, __LINE__))
            return list;
        db.transaction();
        QSqlQuery query(db);
        QHash<int, QString> where;
        where.insert(Constants::INTERACTOR_ISVALID, "=1");
        QString req = ddiCore()->database().select(Constants::Table_INTERACTORS, where);
        if (query.exec(req)) {
            while (query.next()) {
                DrugInteractor *di = new DrugInteractor;
                di->setData(DrugInteractor::Uid, query.value(Constants::INTERACTOR_UID));
                di->setData(DrugInteractor::EnLabel, query.value(Constants::INTERACTOR_EN));
                di->setData(DrugInteractor::FrLabel, query.value(Constants::INTERACTOR_FR));
                di->setData(DrugInteractor::DeLabel, query.value(Constants::INTERACTOR_DE));
                // di->setData(DrugInteractor::EsLabel, query.value(Constants::INTERACTOR_));
                di->setData(DrugInteractor::IsValid, query.value(Constants::INTERACTOR_ISVALID));
                di->setData(DrugInteractor::IsClass, query.value(Constants::INTERACTOR_ISCLASS));
                di->setData(DrugInteractor::ClassInformationFr, query.value(Constants::INTERACTOR_INFO_FR));
                di->setData(DrugInteractor::ClassInformationEn, query.value(Constants::INTERACTOR_INFO_EN));
                di->setData(DrugInteractor::ClassInformationDe, query.value(Constants::INTERACTOR_INFO_DE));
                di->setData(DrugInteractor::DoNotWarnDuplicated, query.value(Constants::INTERACTOR_WARNDUPLICATES));
                di->setData(DrugInteractor::Reference, query.value(Constants::INTERACTOR_REF));
                di->setData(DrugInteractor::PMIDsStringList, query.value(Constants::INTERACTOR_PMIDS));
                di->setData(DrugInteractor::ATCCodeStringList, query.value(Constants::INTERACTOR_ATC));
                di->setData(DrugInteractor::Comment, query.value(Constants::INTERACTOR_COMMENT));
                di->setData(DrugInteractor::IsReviewed, query.value(Constants::INTERACTOR_ISREVIEWED));
                // TODO : di->setData(DrugInteractor::ReviewersStringList, query.value(Constants::INTERACTOR_));
                di->setData(DrugInteractor::DateOfReview, query.value(Constants::INTERACTOR_DATEREVIEW));
                di->setData(DrugInteractor::DateOfCreation, query.value(Constants::INTERACTOR_DATECREATE));
                di->setData(DrugInteractor::DateLastUpdate, query.value(Constants::INTERACTOR_DATEUPDATE));
                di->setData(DrugInteractor::IsAutoFound, query.value(Constants::INTERACTOR_ISAUTOFOUND));
                if (!query.value(Constants::INTERACTOR_CHILDREN).toString().simplified().isEmpty())
                    di->setChildId(query.value(Constants::INTERACTOR_CHILDREN).toStringList());
                list << di;
            }
        } else {
            LOG_QUERY_ERROR_FOR(q, query);
            query.finish();
            db.commit();
            return list;
        }
        query.finish();
        db.commit();
        return list;
    }

public:
    QMultiMap<int, QString> m_iamTreePmids; //K=IAM_ID  ;  V=PMIDs
    QMultiMap<int, QString> m_ddiPmids;     //K=IAK_ID  ;  V=PMIDs

private:
    DrugDatabasePopulator *q;
};
}  // namespace Internal
} // namespace DDI

/*! Constructor of the DrugsDB::Internal::DatabasePopulator class */
DrugDatabasePopulator::DrugDatabasePopulator(QObject *parent) :
    QObject(parent),
    d(new DrugDatabasePopulatorPrivate(this))
{
    setObjectName("DrugDatabasePopulator");
}

/*! Destructor of the DrugsDB::Internal::DatabasePopulator class */
DrugDatabasePopulator::~DrugDatabasePopulator()
{
    if (d)
        delete d;
    d = 0;
}

/*! Initializes the object with the default values. Return true if initialization was completed. */
bool DrugDatabasePopulator::initialize()
{
    return true;
}

/** Save the ATC data to a drug database. Read the XML resource file */
bool DrugDatabasePopulator::saveAtcClassification(DrugsDB::Internal::DrugBaseEssentials *database)
{
    QSqlDatabase db = database->database();
    if (!connectDatabase(db, __FILE__, __LINE__))
        return false;
    LOG("Saving ATC");

    // Clean ATC table from old values
    QString req;
    req = database->prepareDeleteQuery(DrugsDB::Constants::Table_ATC);
    if (!database->executeSQL(req, db))
        return false;
    req = database->prepareDeleteQuery(DrugsDB::Constants::Table_ATC_LABELS);
    if (!database->executeSQL(req, db))
        return false;
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // Import ATC codes to database
//    QFile file(atcCsvFile());
//    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
//        LOG_ERROR(QString("ERROR : can not open file %1, %2.").arg(file.fileName(), file.errorString()));
//    } else {
//        QString content = QString::fromUtf8(file.readAll());
//        if (content.isEmpty())
//            return false;
//        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
//        const QStringList &list = content.split("\n", QString::SkipEmptyParts);
//        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
//        foreach(const QString &s, list) {
//            if (s.startsWith("--")) {
//                qWarning() << s;
//                continue;
//            }
//            QStringList values = s.split("\";\"");
//            QMultiHash<QString, QVariant> labels;
//            QString en = values[1].remove("\"").toUpper();
//            labels.insert("en", en);
//            QString fr = values[2].remove("\"").toUpper();
//            if (fr.isEmpty())
//                labels.insert("fr", en);
//            else
//                labels.insert("fr", fr);
//            QString de = values[3].remove("\"").toUpper();
//            if (de.isEmpty())
//                labels.insert("de", en);
//            else
//                labels.insert("de", de);
//            if (!DrugsDB::Tools::createAtc(database, values[0].remove("\""), labels)) {
//                return false;
//            }
//            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
//        }
//    }
//    // add FreeDiams ATC specific codes
//    db.transaction();

//    // 100 000 < ID < 199 999  == Interacting molecules without ATC code
//    // 200 000 < ID < 299 999  == Interactings classes
//    int molId = 100000;
//    int classId = 200000;
//    foreach(DrugInteractor *di, ddiCore()->getDrugInteractors()) {
//        if (!di->data(DrugInteractor::ATCCodeStringList).toStringList().count()) {
//            // Create new ATC code for mols and/or interacting classes
//            QMultiHash<QString, QVariant> labels;
//            labels.insert("fr", di->data(DrugInteractor::FrLabel));
//            if (!di->data(DrugInteractor::EnLabel).isNull())
//                labels.insert("en", di->data(DrugInteractor::EnLabel));
//            else
//                labels.insert("en", di->data(DrugInteractor::FrLabel));
//            if (!di->data(DrugInteractor::DeLabel).isNull())
//                labels.insert("de", di->data(DrugInteractor::DeLabel));
//            else
//                labels.insert("de", di->data(DrugInteractor::FrLabel));

//            if (di->isClass()) {
//                ++classId;
//                QString n = QString::number(classId-200000);
//                n = n.rightJustified(4, '0');
//                if (!DrugsDB::Tools::createAtc(database, "ZXX" + n, labels, classId, !di->data(DrugInteractor::DoNotWarnDuplicated).toBool()))
//                    return false;
//                di->setData(CLASS_OR_MOL_ID, classId);
//                di->setData(FREEMEDFORMS_ATC_CODE, QString("ZXX" + n));
//            } else {
//                ++molId;
//                QString n = QString::number(molId-100000);
//                n = n.rightJustified(2, '0');
//                if (!DrugsDB::Tools::createAtc(database, "Z01AA" + n, labels, molId, !di->data(DrugInteractor::DoNotWarnDuplicated).toBool()))
//                    return false;
//                di->setData(CLASS_OR_MOL_ID, molId);
//                di->setData(FREEMEDFORMS_ATC_CODE, QString("Z01AA" + n));
//            }
//        }
//        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
//    }

    db.commit();

    LOG("ATC saved");
    return true;
}

/** Populate a drug database with:
 * - the ATC data
 * - DDI data
 * \sa DrugsDB::DrugsDbCore::addInteractionData()
 */
bool DrugDatabasePopulator::saveDrugDrugInteractions(DrugsDB::Internal::DrugBaseEssentials *database)
{
    QSqlDatabase db = database->database();
    if (!connectDatabase(db, __FILE__, __LINE__))
        return false;

    // Save ATC + FreeMedForms specific codes
    if (!database->isAtcAvailable())
        saveAtcClassification(database);

//    const QList<DrugInteractor *> &interactors = ddiCore()->getDrugInteractors();
//    const QList<DrugDrugInteraction *> &ddis   = ddiCore()->getDrugDrugInteractions();
    QList<DDI::DrugInteractor *> interactors = d->getDrugInteractors(database);
    QList<DDI::DrugDrugInteraction *> ddis = d->getDrugDrugInteractions(database);

    LOG(tr("Extracted %1 drug interactors from the %2 database")
        .arg(interactors.count())
        .arg(qApp->applicationName()));

    LOG(tr("Extracted %1 drug-drug interactions from the %2 database")
        .arg(ddis.count())
        .arg(qApp->applicationName()));

    // Recreate interacting classes tree
    LOG("Saving interactors");
    QString req = database->prepareDeleteQuery(DrugsDB::Constants::Table_ATC_CLASS_TREE);//"DELETE FROM ATC_CLASS_TREE";
    database->executeSQL(req, db);
    db.transaction();
    foreach(DDI::DrugInteractor *interactor, interactors) {
        if (interactor->isClass()) {
            if (!d->saveClassDrugInteractor(interactor, interactors, database, 0)) {
                db.rollback();
                return false;
            }
        }
    }
    db.commit();

    // Save DDIs
    d->saveDrugDrugInteractions(database, interactors, ddis);

    // Save Bibliographic references
    d->saveBibliographicReferences(database);

    // refresh the innToAtc content (reload ATC codes because we added some new ATC)

    // drugs databases needs to be relinked with the new ATC codes

    return true;
}

