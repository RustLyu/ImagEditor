#ifndef SELECTVIEWSIZEDIALOG_H
#define SELECTVIEWSIZEDIALOG_H

#include <QDialog>

namespace Ui {
	class selectViewSizeDialog;
}

class selectViewSizeDialog : public QDialog
{
	Q_OBJECT

public:
	explicit selectViewSizeDialog(QWidget *parent = 0);
	~selectViewSizeDialog();

private slots:
	void on_m_recoverBtn_clicked();

	void on_m_okBtn_clicked();

	void on_m_cancleBtn_clicked();

private:
	Ui::selectViewSizeDialog *ui;
	void clear();
public:
	void fillAttr(const QPoint start, const double width, const double height);
protected:
	void closeEvent(QCloseEvent *);
};

#endif // SELECTVIEWSIZEDIALOG_H
