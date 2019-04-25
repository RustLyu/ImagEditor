#include <QDateTime>
#include <QFileDialog>

#include "config.h"
#include "savetopimgdialog.h"
#include "ui_savetopimgdialog.h"

extern QString g_backPngPath;

SaveTopImgDialog::SaveTopImgDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SaveTopImgDialog)
{
	ui->setupUi(this);
	initUI();
	registSignal2Slots();	
}

SaveTopImgDialog::~SaveTopImgDialog()
{
	delete ui;
}

void SaveTopImgDialog::initUI()
{
	ui->buttonBox->addButton(tr("确定"), QDialogButtonBox::AcceptRole);
	ui->buttonBox->addButton(tr("取消"), QDialogButtonBox::RejectRole);
	QStringList items;
	items << "Red" << "Green" << "Blue" << "RGBA";
	ui->m_selectImgType->addItems(items);
}

void SaveTopImgDialog::registSignal2Slots()
{
	connect(ui->m_selectSavePathBtn, SIGNAL(clicked()), this, SLOT(Slot_on_m_selectSvePathBtn_clicked()));
}

void SaveTopImgDialog::Slot_on_m_selectSvePathBtn_clicked()
{
	QString name = ui->m_saveTopImgPath->text();
	QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存图片"), name, "Files (*bmp)");
	ui->m_saveTopImgPath->setText(path);
}

std::tuple<QString, QString> SaveTopImgDialog::getSaveAAttribute()
{
	QString path = nullptr;
	QString color = nullptr;
	if (this->exec() == QDialog::Accepted)
	{
		path = ui->m_saveTopImgPath->text();
		color = ui->m_selectImgType->currentText();
	}
	return std::make_tuple(path, color);
}

std::tuple<QString, QString> SaveTopImgDialog::getDefaultAAttribute()
{
	return std::make_tuple(ui->m_saveTopImgPath->text(), ui->m_selectImgType->currentText());
}

void SaveTopImgDialog::setSaveImagPath(const QString path)
{
	ui->m_saveTopImgPath->setText(path);
}
