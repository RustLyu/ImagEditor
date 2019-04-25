#include <Qdebug>
#include <QMessageBox>

#include "setmiledialog.h"
#include "svgviewshow.h"
#include "mainwindow.h"
#include "config.h"
#include "ui_setmiledialog.h"

SetMileDialog::SetMileDialog(QWidget *parent) :QDialog(parent), ui(new Ui::SetMileDialog)
{
	this->setWindowTitle(QL("编辑里程"));
	ui->setupUi(this);
	initUI();
	registerSignal2Slots();
}

SetMileDialog::~SetMileDialog()
{
	delete ui;
}

void SetMileDialog::initUI()
{
	ui->m_isScale->setChecked(true);

	// 整数部分0-4位，小数部分0-5位
    QRegExp reg("[0-9]{0,4}[\.][0-9]{0,5}");
    //QRegExp reg("[0-9]{0,2}");
	ui->m_startEdit->setValidator(new QRegExpValidator(reg, this));
	ui->m_endEdit->setValidator(new QRegExpValidator(reg, this));
	ui->m_startEdit->setToolTip(QL("起始里程"));
	ui->m_endEdit->setToolTip(QL("终止里程"));
	ui->m_isScale->setVisible(false);
	ui->m_unit->addItem(QL("公里"));
	ui->m_unit->addItem(QL("里"));
}

void SetMileDialog::registerSignal2Slots()
{
	connect(ui->m_saveBtn, SIGNAL(clicked()), this, SLOT(SLOT_OnSaveBtnClick()));
	connect(ui->m_deleteBtn, SIGNAL(clicked()), this, SLOT(SLOT_OnDeleteBtnClick()));
}

void SetMileDialog::SLOT_OnSaveBtnClick()
{
	qreal start = ui->m_startEdit->text().toDouble();
	qreal end = ui->m_endEdit->text().toDouble();
	if (!start || !end)
	{
		QMessageBox megBox(QMessageBox::Warning, QL("警告"), QL("起始公里和终止里程不能为空!"), QMessageBox::Yes, this);
		megBox.setButtonText(QMessageBox::Yes, QL("确定"));
		megBox.exec();
		return;
	}
	if (end <= start)
	{
		QMessageBox megBox(QMessageBox::Warning, QL("警告"), QL("终点必须大于起点!"), QMessageBox::Yes, this);
		megBox.setButtonText(QMessageBox::Yes, QL("确定"));
		megBox.exec();
		return;
	}
	// 标记刻度
	if (ui->m_isScale->checkState() == Qt::Checked)
	{
		SvgViewShow::Instance->saveMile(start, end, ui->m_unit->currentText(), true);
	}
	// 不标记刻度
	else
	{
		SvgViewShow::Instance->saveMile(start, end, ui->m_unit->currentText(), false);
	}
	this->close();
}

void SetMileDialog::SLOT_OnDeleteBtnClick()
{
	SvgViewShow::Instance->clearMileFlag();
	SvgViewShow::Instance->removeMileData();
	MainWindow::Instance->setStatusMessage(QL("开始移除里程标记,请稍等"));
	this->close();
}

void SetMileDialog::clear()
{
	ui->m_startEdit->clear();
	ui->m_endEdit->clear();
}

void SetMileDialog::setStartMile(const QString start)
{
	ui->m_startEdit->setText(start);
}

// 设置里程单位
void SetMileDialog::setUnitText(const QString unit)
{
	ui->m_unit->setCurrentText(unit);
}

void SetMileDialog::setEndMile(const QString end)
{
	ui->m_endEdit->setText(end);
}

void SetMileDialog::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	SvgViewShow::Instance->clearMilePos();
}
