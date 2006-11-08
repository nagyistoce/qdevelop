#include "findfileimpl.h"
#include <QFileDialog>
#include <QDir>
#include <QListWidget>
#include <QTextStream>
#include <QDebug>
//
FindFileImpl::FindFileImpl( QWidget * parent, QStringList directories, QListWidget *listResult, QListWidget *findLines) 
	: QDialog(parent), m_parent(parent), m_listResult(listResult), m_listLines(findLines)
{
	setupUi(this);
	for(int i=0; i<directories.count(); i++)
		comboFindIn->addItem(directories.at(i)+"/");
}
//
void FindFileImpl::on_chooseDirectoryButton_clicked()
{
	QString s = QFileDialog::getExistingDirectory(
		m_parent,
		tr("Choose a directory"),
		QDir::homePath(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	comboFindIn->addItem(s);
	comboFindIn->setCurrentIndex( comboFindIn->count()-1 );
}
//
void FindFileImpl::on_findButton_clicked()
{
	if( !comboFindIn->count() || textFind->currentText().isEmpty() )
		return;
	if( findButton->text() == tr("&Find") )
	{
		findButton->setText(tr("&Stop"));
		closeButton->setEnabled( false );
		m_stop = false;
		m_listResult->clear();
		m_listLines->clear();
	}
	else
	{
		findButton->setText(tr("&Find"));
		closeButton->setEnabled( true );
		m_stop = true;
	}
	find( comboFindIn->currentText() );
	if( m_listResult->count() )
	{
		m_listResult->setItemSelected( m_listResult->item(0), true);
		m_listLines->addItems( m_listResult->item(0)->data(Qt::UserRole).toStringList() );		
	}
	findButton->setText(tr("&Find"));
	closeButton->setEnabled( true );
}
//
void FindFileImpl::find( QString directory )
{
	QDir dir(directory);
	QString filterNames = comboFileTypes->currentText();
	filterNames.remove(" ");
	QFileInfoList list = dir.entryInfoList(filterNames.split(";"), QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	foreach(QFileInfo fileInfo, list)
	{
		qApp->processEvents();
		if( m_stop )
			return;
		if( fileInfo.isFile() )
		{
			findInFile( fileInfo.absoluteFilePath() );
		}
		else if( checkRecursive->isChecked() )// Directory
		{
			find( fileInfo.absoluteFilePath() );
		}
	}
}
//
void FindFileImpl::findInFile( QString filename )
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	    return;
	   //
	QRegExp exp;
	if( checkWholeWords->isChecked() )
		exp.setPattern( "\\b"+textFind->currentText()+"\\b");
	else
		exp.setPattern( textFind->currentText());
	exp.setCaseSensitivity((Qt::CaseSensitivity) checkCase->isChecked());
	
	//
	QTextStream in(&file);
	int numLine = 0;
	QStringList lines;
	while (!in.atEnd()) 
	{
		numLine++;
	    QString line = in.readLine();
	    if( line.contains(exp) )
	    {
	    	lines << tr("Line")+" "+QString::number(numLine)+" : "+line;
	    }
	}
	file.close();
	if( lines.count() )
	{
		m_listResult->addItem( tr("File")+" : "+filename );
		QListWidgetItem *item = m_listResult->item(m_listResult->count()-1);
		item->setData(Qt::UserRole, QVariant(lines) );
	}
}
//