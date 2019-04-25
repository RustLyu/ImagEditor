#ifndef SAVETOPIMGDIALOG_H
#define SAVETOPIMGDIALOG_H

#include <QDialog>

namespace Ui {
class SaveTopImgDialog;
}

class SaveTopImgDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SaveTopImgDialog(QWidget *parent = 0);
	~SaveTopImgDialog();

	std::tuple<QString, QString> getSaveAAttribute();
	std::tuple<QString, QString> getDefaultAAttribute();
	void setSaveImagPath(const QString path);

private slots:
	void Slot_on_m_selectSvePathBtn_clicked();

private:
	Ui::SaveTopImgDialog *ui;
private:
	void initUI();
	void registSignal2Slots();
};

#endif // SAVETOPIMGDIALOG_H
