#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#define QL(a) QStringLiteral(a)

class Config
{
private:
	explicit Config();
	~Config();
public:
	static Config* Instance;
private:
	QString m_topSvgPath;
	QString m_backPngPath;
	QString m_helpPath;
	qreal   m_stepLen;
	QString m_crossPicPath;
	QString m_flagPicPath;
	QString m_textPosPicPath;
	QString m_showConfigPath;
public:
	void	SetBackPngPath(const QString path);
	void	SetTopSvgPath(const QString path);
	QString GetTopSvgPath() const;
	QString GetBackPngPath() const;
	QString GetHelpPath() const;
	qreal   GetStepLen() const;
	QString GetFlagPicPath() const;
	QString GetTextPosPicPath() const;
	QString GetCrossPicPath() const;
	QString GetShowConfigPath() const;

};

#endif // CONFIG_H
