#include "config.h"

Config* Config::Instance = new Config();
Config::Config()
{
	m_backPngPath = nullptr;
	// 顶图svg路径
	m_topSvgPath = nullptr;
	m_helpPath = QStringLiteral("./使用说明.txt");
	m_stepLen = 0;
	m_crossPicPath = "://Resource/cross.png";
	m_flagPicPath = "://Resource/flag.png";
	m_textPosPicPath = "://Resource/TextPos.png";
	m_showConfigPath = "./actionVisible.json";
}

Config::~Config()
{
}

void Config::SetTopSvgPath(const QString path)
{
	m_topSvgPath = path;
}

QString Config::GetTopSvgPath() const
{
	return m_topSvgPath;
}

void Config::SetBackPngPath(const QString path)
{
	m_backPngPath = path;
}

QString Config::GetBackPngPath() const
{
	return m_backPngPath;
}

QString Config::GetHelpPath() const
{
	return m_helpPath;
}

qreal Config::GetStepLen() const
{
	return m_stepLen;
}

QString Config::GetFlagPicPath() const
{
	return m_flagPicPath;
}

QString Config::GetTextPosPicPath() const
{
	return m_textPosPicPath;
}

QString Config::GetCrossPicPath() const
{
	return m_crossPicPath;
}

QString Config::GetShowConfigPath() const
{
	return m_showConfigPath;
}