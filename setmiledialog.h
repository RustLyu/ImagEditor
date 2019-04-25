#ifndef SETMILEDIALOG_H
#define SETMILEDIALOG_H

#include <QDialog>

namespace Ui {
	class SetMileDialog;
}

class SetMileDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SetMileDialog(QWidget *parent = 0);
	~SetMileDialog();
	void clear();
	void setStartMile(const QString start);
	void setEndMile(const QString end);
	void setUnitText(const QString unit);
private:
	Ui::SetMileDialog *ui;
	private slots:
	void SLOT_OnSaveBtnClick();
	void SLOT_OnDeleteBtnClick();
protected:
	void closeEvent(QCloseEvent *event);
private:
	void initUI();
	void registerSignal2Slots();
};

#endif // SETMILEDIALOG_H
