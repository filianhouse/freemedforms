/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  The FreeAccount plugins are free, open source FreeMedForms' plugins.   *
 *  (C) 2010-2011 by Pierre-Marie Desombre, MD <pm.desombre@medsyn.fr>     *
 *  and Eric Maeker, MD <eric.maeker@gmail.com>                            *
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
 *  Main Developpers : Pierre-Marie DESOMBRE <pm.desombre@medsyn.fr>,      *
 *                     Eric MAEKER, <eric.maeker@gmail.com>                *
 *  Contributors :                                                         *
 *      NAME <MAIL@ADRESS>                                                 *
 ***************************************************************************/
#include "ledgeredit.h"
#include "ui_ledgeredit.h"
#include "ledgerIO.h"
#include <coreplugin/idocumentprinter.h>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/constants_tokensandsettings.h>
#include <utils/log.h>
#include <QDebug>
enum { WarnDebugMessage = true };
using namespace ExtensionSystem;
using namespace Core;
using namespace Core::Constants;
inline static Core::IDocumentPrinter *printer() {return ExtensionSystem::PluginManager::instance()
                                                 ->getObject<Core::IDocumentPrinter>();}

LedgerEdit::LedgerEdit(QWidget * parent):QWidget(parent),ui(new Ui::LedgerEditWidget){
    ui->setupUi(this);
    int h = parent->height();
    int w = parent->width();
    resize(w,h);
    setAutoFillBackground(true);
    LedgerIO lio(this);
    QStringList listOfYears;
    QString currentDate = QDate::currentDate().toString("yyyy");
    listOfYears << currentDate;
    listOfYears << lio.getListOfYears();
    qDebug() << __FILE__ << QString::number(__LINE__) << " listOfYears.size() =" <<QString::number(listOfYears.size())  ;
    listOfYears.removeDuplicates();
    qDebug() << __FILE__ << QString::number(__LINE__) << " listOfYears.size() =" <<QString::number(listOfYears.size())  ;
    for (int i = 0; i < listOfYears.size(); i += 1)
    {
    	//if (WarnDebugMessage)
    	    	qDebug() << __FILE__ << QString::number(__LINE__) << " listOfYears[i] =" << listOfYears[i] ;
        }
    ui->yearComboBox->addItems(listOfYears);
    ui->infoLabel->setText("");
    emit choosenDate(currentDate);
    m_doc = new QTextDocument(ui->textEdit);
    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Base, QColor ("#DDDDDD"));
    ui->textEdit->setPalette(p);
    ui->textEdit->setDocument(m_doc);
    m_myThread = new ProduceDoc();
    //print
    m_typeOfPaper = 0;
    m_duplicata = false;
    connect(ui->quitButton,SIGNAL(pressed()),this,SLOT(close()));
    connect(ui->showButton,SIGNAL(pressed()),this,SLOT(showLedger()));
    connect(ui->printButton,SIGNAL(pressed()),this,SLOT(printLedger()));
    connect(ui->yearComboBox,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(choosenDate(const QString &)));
}

LedgerEdit::~LedgerEdit(){
    delete m_myThread;
    delete m_doc;
}

void LedgerEdit::showLedger(){
    //qDebug() << __FILE__ << QString::number(__LINE__) << " SHOW !!! "  ;
    //m_doc->clear();
    m_myThread->dateChosen(m_date);
    if (m_myThread->isRunning())
    {
    	  m_myThread->terminate();
    	  if (WarnDebugMessage)
    	      	  qDebug() << __FILE__ << QString::number(__LINE__) << " in  m_myThread->terminate"   ;
        }
    m_myThread->start();
    connect(m_myThread ,SIGNAL(finished()),this,SLOT(getDocument()));
    connect(m_myThread ,SIGNAL(outThread(const QString &)),this,       SLOT(fillInfoLabel(const QString &)));
    connect(m_myThread ,SIGNAL(started()),                 this,       SLOT(inThread()));
    connect(this       ,SIGNAL(deleteThread()),            this,       SLOT(slotDeleteThread()));
}

void LedgerEdit::printLedger(){
    Core::IDocumentPrinter *p = printer();
    if (!p) {
        qWarning() << __FILE__ << QString::number(__LINE__) << "No IDocumentPrinter found" ;
        LOG_ERROR("No IDocumentPrinter found");
        return;
    }
    p->clearTokens();
    QHash<QString, QVariant> tokens;

    // Là tu ajoutes tes tokens pour l'impression (lire la doc de FreeDiams sur le gestionnaire d'étiquettes)
    tokens.insert(Core::Constants::TOKEN_DOCUMENTTITLE, this->windowTitle());
    p->addTokens(Core::IDocumentPrinter::Tokens_Global, tokens);
    
    // Ensuite on demande l'impression (avec les entêtes/pieds de page et duplicata ou non)
    p->print(m_doc, m_typeOfPaper, m_duplicata);    
}

void LedgerEdit::choosenDate(const QString & dateText){
    ////qDebug() << __FILE__ << QString::number(__LINE__) << " dateText =" << dateText ;
    m_date = QDate::fromString(dateText,"yyyy");
}

void LedgerEdit::fillInfoLabel(const QString & textFromThread){
    ui->infoLabel->setText(textFromThread);
}

void LedgerEdit::inThread(){
  m_myThread->dateChosen(m_date);
}

void LedgerEdit::getDocument(){
    ////qDebug() << __FILE__ << QString::number(__LINE__) << " getDocument " ;
    m_doc = m_myThread->getTextDocument()->clone();
    emit  deleteThread();
    ui->textEdit->setDocument(m_doc);
}

void LedgerEdit::slotDeleteThread(){
    if (m_myThread)
    {
        delete m_myThread;    	  
        }

    m_myThread = new ProduceDoc(); 
}

void LedgerEdit::resizeLedgerEdit(QWidget * parent){
    int h = parent->height();
    int w = parent->width();
    resize(w,h);
}
