#include "ui_selectviewsizedialog.h"
#include "selectviewsizedialog.h"

selectViewSizeDialog::selectViewSizeDialog(QWidget *parent) :
QDialog(parent),
ui(new Ui::selectViewSizeDialog)
{
	ui->setupUi(this);
}

selectViewSizeDialog::~selectViewSizeDialog()
{
	delete ui;
}

void selectViewSizeDialog::on_m_recoverBtn_clicked()
{
	close();
}

void selectViewSizeDialog::on_m_okBtn_clicked()
{
	close();
}

void selectViewSizeDialog::on_m_cancleBtn_clicked()
{
	close();
}

void selectViewSizeDialog::fillAttr(const QPoint start, const double width, const double height)
{
	ui->m_startX->setText(QString::number(start.x()));
	ui->m_startY->setText(QString::number(start.y()));
	ui->m_endX->setText(QString::number(start.x() + width));
	ui->m_endY->setText(QString::number(height));
}

void selectViewSizeDialog::clear()
{
	ui->m_startX->clear();
	ui->m_startY->clear();
	ui->m_endX->clear();
	ui->m_endY->clear();
}

void selectViewSizeDialog::closeEvent(QCloseEvent *)
{
	clear();
}