#include <QDebug>
#include <QDomDocument>
#include <QFileInfo>

#include "xmlnodeaction.h"
#include "config.h"

const XmlNodeAction* XmlNodeAction::Instance = new XmlNodeAction();
XmlNodeAction::XmlNodeAction()
{
}

bool XmlNodeAction::loadXml(QList<TipsWithPos> &itemList)
{
	itemList.clear();
	QFile file(Config::Instance->GetTopSvgPath());
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		file.close();
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		return false;
	}
	QDomElement userNode = root.firstChildElement();

	while (!userNode.isNull())
	{
		std::string str_g = userNode.nodeName().toStdString();
		if (str_g.compare("g") == 0)
		{
			QDomElement child = userNode.firstChildElement();
			TipsWithPos cell;
			cell.id = userNode.attributeNode("id").value();
			cell.strokeWidth = userNode.attributeNode("stroke-width").value();
			if (userNode.attributeNode("mile").value().toInt())
			{
				cell.isMile = true;
			}
			while (!child.isNull())
			{
				std::string str_c = child.nodeName().toStdString();
				if (str_c.compare("polygon") == 0)
				{
					QString value = child.attributeNode("points").value();
					QStringList posList = value.split(" ");
					for (auto pos : posList)
					{
						TipsWithPos::Point point;
						QStringList posCell = pos.split(",");
						if (posCell.count() != 2)
						{
							qDebug() << "Polygon xml analyse error";
							continue;
						}
						point.x = posCell[0].toDouble();
						point.y = posCell[1].toDouble();
						cell.points.push_back(point);
					}
					cell.type = EditType::EditType_Polygon;
				}
				else if (str_c.compare("polyline") == 0)
				{
					QString value = child.attributeNode("points").value();
					QStringList posList = value.split(" ");
					for (auto pos : posList)
					{
						TipsWithPos::Point point;
						QStringList posCell = pos.split(",");
						if (posCell.count() != 2)
						{
							qDebug() << "polyline xml analyse error";
							continue;
						}
						point.x = posCell[0].toDouble();
						point.y = posCell[1].toDouble();
						cell.points.push_back(point);
					}
					cell.type = EditType::EditType_Line;
				}
				else if (str_c.compare("Tips") == 0)
				{
					cell.content = child.attributeNode("content").value();
				}
				else if (str_c.compare("Color") == 0)
				{
					QStringList colorCell = child.attributeNode("color").value().split(",");
					if (colorCell.count() != 4)
					{
						qDebug() << "Color xml analyse error";
						continue;
					}
					cell.color.setRed(colorCell[0].toInt());
					cell.color.setGreen(colorCell[1].toInt());
					cell.color.setBlue(colorCell[2].toInt());
					cell.color.setAlpha(colorCell[3].toInt());
				}
				else if (str_c.compare("text") == 0)
				{
					cell.content = child.text();
					cell.type = EditType::EditType_Text;
					QStringList attr = child.attributeNode("transform").value().split("(")[1].split(")")[0].split(" ");
					if (attr.count() != 6)
						continue;
					TipsWithPos::Point point;
					point.x = attr[4].toDouble();
					point.y = attr[5].toDouble();
					cell.points.push_back(point);
					cell.front = child.attributeNode("font-size").value().toDouble();
				}
				child = child.nextSiblingElement();
			}
			itemList.push_back(cell);
		}
		userNode = userNode.nextSiblingElement();
	}
	return true;
}

bool XmlNodeAction::create(const QString fileName, const QSize size)
{
	QFile file(fileName);
	file.open(QIODevice::ReadWrite);
	QDomDocument doc;
	QDomProcessingInstruction instruction;
	instruction = doc.createProcessingInstruction("xml", "version=\'1.0\' standalone=\'no\'");
	doc.appendChild(instruction);
	QDomElement root = doc.createElement("svg");

	std::map<QString, QString> nodeAttr;
	nodeAttr["xmlns:xlink"] = "http://www.w3.org/1999/xlink";
	nodeAttr["version"] = "1.1";
	nodeAttr["xmlns"] = "http://www.w3.org/2000/svg";
	nodeAttr["fill-rule"] = "evenodd";
	nodeAttr["stroke-linejoin"] = "round";
	nodeAttr["xml:space"] = "preserve";
	nodeAttr["stroke-linecap"] = "round";
	nodeAttr["viewBox"] = "0 0 " + QString::number(size.width()) + " " + QString::number(size.height());
	for (auto it : nodeAttr)
	{
		QDomAttr node_key = doc.createAttribute(it.first);
		node_key.setValue(it.second);
		root.setAttributeNode(node_key);
	}

	doc.appendChild(root);
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

bool XmlNodeAction::SLOT_Add(const QPoint &start, const QPoint &end, const qreal startMile, const QSize size)
{
	if (start.x() == 0 || end.x() == 0 || startMile < 0)
		return false;
	QFile file;
	QDomDocument doc;
	if (!getFileHandle(doc, file, size))
	{
		return false;
	}
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		root = doc.createElement("svg");
	}

	uint id;
	if (root.lastChild().isNull())
	{
		id = 0;
	}
	else
	{
		id = root.lastChild().toElement().attribute("id").toUInt();
	}
	qreal stepLen = Config::Instance->GetStepLen();
	uint all = (end.x() - start.x()) / stepLen;

	for (uint index = 0; index <= all; ++index)
	{
		++id;
		QDomElement element_root = doc.createElement("g");

		std::map<QString, QString> nodeAttr;
		nodeAttr["stroke-width"] = "0";
		nodeAttr["fill"] = "rgb(0,0,0)";
		//nodeAttr["stroke"] = "rgb(255,255,0)";
		nodeAttr["clip-path"] = "1";
		nodeAttr["id"] = QString::number(id);
		nodeAttr["mile"] = "1";

		QDomElement element_children = doc.createElement("text");
		std::map<QString, QString> childNodeAttr;
		childNodeAttr["transform"] = "matrix(14.6042 0 -0 7.30212 " + QString::number(start.x() + stepLen * index) + " " + QString::number(start.y()) + ")";
		childNodeAttr["font-family"] = "SimSun";
		childNodeAttr["font-size"] = "9";

		uint mile = startMile + index;
		QDomText text = doc.createTextNode(mile % 10 ? QString::number(mile % 10) : ("YDK" + QString::number(mile / 10)));
		element_children.appendChild(text);

		for (auto it : childNodeAttr)
		{
			QDomAttr node_Key = doc.createAttribute(it.first);
			node_Key.setValue(it.second);
			element_children.setAttributeNode(node_Key);
		}
		element_root.appendChild(element_children);

		for (auto it : nodeAttr)
		{
			QDomAttr node_Key = doc.createAttribute(it.first);
			node_Key.setValue(it.second);
			element_root.setAttributeNode(node_Key);
		}
		root.appendChild(element_root);
	}

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

bool XmlNodeAction::SLOT_Add(const QPointF &point, const NodeAttribute attr, const QSize size, TextTagType textType)
{
	if (attr.content.isEmpty())
		return false;
	QDomDocument doc;
	QFile file;
	if (!getFileHandle(doc, file, size))
	{
		return false;
	}
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		root = doc.createElement("svg");
	}

	QString str_id;
	if (root.lastChild().isNull())
	{
		str_id = "0";
	}
	else
	{
		str_id = root.lastChild().toElement().attribute("id");
	}

	QDomElement element_root = doc.createElement("g");

	std::map<QString, QString> nodeAttr;
	nodeAttr["stroke-width"] = "0";
	nodeAttr["fill"] = "rgb(" + attr.red + "," + attr.green + "," + attr.blue + ")";
	//nodeAttr["stroke"] = "rgb(255,255,0)";
	nodeAttr["clip-path"] = "1";
	nodeAttr["id"] = QString::number(str_id.toInt() + 1);
	nodeAttr["mile"] = QString::number(textType);

	QDomElement element_children = doc.createElement("text");
	QDomElement element_color = doc.createElement("Color");
	std::map<QString, QString> childNodeAttr;
	childNodeAttr["transform"] = "matrix(14.6042 0 -0 7.30212 " + QString::number(point.x()) + " " + QString::number(point.y()) + ")";
	childNodeAttr["font-family"] = "SimSun";
	childNodeAttr["font-size"] = attr.front;

	QDomText text = doc.createTextNode(attr.content);
	element_children.appendChild(text);

	QDomAttr colorAttr = doc.createAttribute("color");
	colorAttr.setValue(attr.red + "," + attr.green + "," + attr.blue + "," + attr.alpha);

	for (auto it : childNodeAttr)
	{
		QDomAttr node_Key = doc.createAttribute(it.first);
		node_Key.setValue(it.second);
		element_children.setAttributeNode(node_Key);
	}
	element_root.appendChild(element_children);

	// 颜色
	element_color.setAttributeNode(colorAttr);
	if (!textType)
		element_root.appendChild(element_color);

	for (auto it : nodeAttr)
	{
		QDomAttr node_Key = doc.createAttribute(it.first);
		node_Key.setValue(it.second);
		element_root.setAttributeNode(node_Key);
	}
	root.appendChild(element_root);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

bool XmlNodeAction::SLOT_Add(const QVector<QPoint> &points, const NodeAttribute attr, const QSize size)
{
	QFile file;
	QDomDocument doc;
	if (!getFileHandle(doc, file, size))
	{
		return false;
	}
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		root = doc.createElement("svg");
	}

	QString str_id;
	if (root.lastChild().isNull())
	{
		str_id = "0";
	}
	else
	{
		str_id = root.lastChild().toElement().attribute("id");
	}

	QDomElement element_root = doc.createElement("g");

	std::map<QString, QString> nodeAttr;
	nodeAttr["stroke-width"] = "1";
	nodeAttr["fill"] = "rgb(" + attr.red + "," + attr.green + "," + attr.blue + ")";
	//nodeAttr["stroke"] = "rgb(255,255,0)";
	nodeAttr["clip-path"] = "1";
	nodeAttr["id"] = QString::number(str_id.toInt() + 1);
	nodeAttr["mile"] = "0";

	QDomElement element_children = doc.createElement("polygon");
	QString attr_children;

	QDomElement element_content = doc.createElement("Tips");

	QDomElement element_color = doc.createElement("Color");
	for (int index = 0; index < points.count(); ++index)
	{
		QPoint point = points[index];
		attr_children.append(QString::number(point.x()) + "," + QString::number(point.y()));
		if (index < (points.count() - 1))
		{
			attr_children.append(" ");
		}
	}
	QDomAttr childrenAttr = doc.createAttribute("points");
	childrenAttr.setValue(attr_children);

	QDomAttr contentAttr = doc.createAttribute("content");
	contentAttr.setValue(attr.content);

	QDomAttr colorAttr = doc.createAttribute("color");
	colorAttr.setValue(attr.red + "," + attr.green + "," + attr.blue + "," + attr.alpha);

	// 点集合
	element_children.setAttributeNode(childrenAttr);
	element_root.appendChild(element_children);

	// Tips
	element_content.setAttributeNode(contentAttr);
	element_root.appendChild(element_content);

	// 颜色
	element_color.setAttributeNode(colorAttr);
	element_root.appendChild(element_color);

	for (auto it : nodeAttr)
	{
		QDomAttr node_Key = doc.createAttribute(it.first);
		node_Key.setValue(it.second);
		element_root.setAttributeNode(node_Key);
	}
	root.appendChild(element_root);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

// 更新节点
bool XmlNodeAction::SLOT_Update(const QString nodeId, const NodeAttribute attr)
{
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	QFile file(topSvgPath);
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		file.close();
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		return false;
	}

	QDomElement userNode = root.firstChildElement();
	std::string rootname = root.nodeName().toStdString();
	bool find = false;
	while (!userNode.isNull() && !find)
	{
		std::string str = userNode.nodeName().toStdString();
		if (str.compare("g") == 0)
		{
			std::string str_c = userNode.attributeNode("id").value().toStdString();
			if (str_c.compare(nodeId.toStdString()) == 0)
			{
				userNode.attributeNode("fill").setValue("rgb(" + attr.red + "," + attr.green + "," + attr.blue + ")");
				userNode.attributeNode("stroke-width").setValue(attr.strokeWidth);
				userNode.attributeNode("stroke").setValue("rgb(" + attr.red + "," + attr.green + "," + attr.blue + ")");
				QDomElement child = userNode.firstChildElement();
				while (!child.isNull() && !find)
				{
					std::string str = child.nodeName().toStdString();
					if (str.compare("Tips") == 0)
					{
						QDomElement element_content = doc.createElement("Tips");
						QDomAttr contentAttr = doc.createAttribute("content");
						contentAttr.setValue(attr.content);
						element_content.setAttributeNode(contentAttr);
						userNode.replaceChild(element_content, child);
						child = element_content.nextSiblingElement();
					}
					else if (str.compare("Color") == 0)
					{
						QDomElement element_color = doc.createElement("Color");
						QDomAttr colorAttr = doc.createAttribute("color");
						colorAttr.setValue(attr.red + "," + attr.green + "," + attr.blue + "," + attr.alpha);
						element_color.setAttributeNode(colorAttr);
						userNode.replaceChild(element_color, child);
						child = element_color.nextSiblingElement();
					}
					else if (str.compare("text") == 0)
					{
						QDomNode oldnode = child.firstChild();
						child.firstChild().setNodeValue(attr.content);
						QDomNode newnode = child.firstChild();
						child.attributeNode("font-size").setValue(attr.front);
						child.replaceChild(newnode, oldnode);
						child = child.nextSiblingElement();
					}
					else
						child = child.nextSiblingElement();
				}
				find = true;
			}
			userNode = userNode.nextSiblingElement();
		}
	}

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

// 移除节点
bool XmlNodeAction::SLOT_Remove(const QString nodeId)
{
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	QFile file(topSvgPath);
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		file.close();
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		return false;
	}

	QDomElement userNode = root.firstChildElement();
	bool find = false;
	while (!userNode.isNull() && !find)
	{
		qDebug() << userNode.nodeName();
		if (userNode.nodeName().toStdString().compare("g") == 0)
		{
			if (userNode.attributeNode("id").value().toStdString().compare(nodeId.toStdString()) == 0)
			{
				root.removeChild(userNode);
				find = true;
			}
			userNode = userNode.nextSiblingElement();
		}
	}

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

// 获取新item编号
QString XmlNodeAction::getNewID()
{
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	if (!QFileInfo::exists(topSvgPath))
	{
		return "1";
	}
	QFile file(topSvgPath);
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		return "1";
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return "1";
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		root = doc.createElement("svg");
	}

	QString str_id;
	if (root.lastChild().isNull())
	{
		str_id = "0";
	}
	else
	{
		str_id = root.lastChild().toElement().attribute("id");
	}
	file.close();
	return QString::number(str_id.toInt() + 1);
}

// 移除节点
bool XmlNodeAction::SLOT_Remove(const QList<QString> tmp)
{
	QList<QString> nodeIdList;
	for (auto value : tmp)
	{
		nodeIdList.push_back(value);
	}
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	QFile file(topSvgPath);
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		file.close();
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		return false;
	}

	QDomElement userNode = root.firstChildElement();
	while (!userNode.isNull() && !nodeIdList.isEmpty())
	{
		if (userNode.nodeName().toStdString().compare("g") == 0)
		{
			QString str_c = userNode.attributeNode("id").value();
			QDomElement next = userNode.nextSiblingElement();;
			if (nodeIdList.contains(str_c))
			{
				root.removeChild(userNode);
				nodeIdList.removeOne(str_c);
			}
			userNode = next;
		}
	}

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}

bool XmlNodeAction::SLOT_Add(const QVector<QPoint> &points, const NodeAttribute attr, const QSize size, double strokeWidth)
{
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	Q_UNUSED(strokeWidth);
	QFile file;
	QDomDocument doc;
	if (!getFileHandle(doc, file, size))
	{
		return false;
	}
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		root = doc.createElement("svg");
	}

	QString str_id;
	if (root.lastChild().isNull())
	{
		str_id = "0";
	}
	else
	{
		str_id = root.lastChild().toElement().attribute("id");
	}

	QDomElement element_root = doc.createElement("g");
	QDomElement element_color = doc.createElement("Color");

	std::map<QString, QString> nodeAttr;
	nodeAttr["stroke-width"] = attr.strokeWidth;
	nodeAttr["fill"] = "rgb(" + attr.red + "," + attr.green + "," + attr.blue + ")";
	//nodeAttr["stroke"] = "rgb(" + attr.red + "," + attr.green + "," + attr.blue + ")";
	nodeAttr["clip-path"] = "1";
	nodeAttr["id"] = QString::number(str_id.toInt() + 1);
	nodeAttr["mile"] = "0";

	int count = points.count();
	for (int index = 0; index < count; ++index)
	{
		QDomElement element_children = doc.createElement("polyline");
		QString attr_children;
		QPoint start = points[index];
		QPoint end = points[0];
		if ((index + 1) < count)
		{
			end = points[index + 1];
		}

		attr_children.append(QString::number(start.x()) + "," + QString::number(start.y()) + " " + QString::number(end.x()) + "," + QString::number(end.y()));
		QDomAttr childrenAttr = doc.createAttribute("points");
		childrenAttr.setValue(attr_children);
		// 点集合
		element_children.setAttributeNode(childrenAttr);
		element_root.appendChild(element_children);
	}

	for (auto it : nodeAttr)
	{
		QDomAttr node_Key = doc.createAttribute(it.first);
		node_Key.setValue(it.second);
		element_root.setAttributeNode(node_Key);
	}

	QDomAttr colorAttr = doc.createAttribute("color");
	colorAttr.setValue(attr.red + "," + attr.green + "," + attr.blue + "," + attr.alpha);
	// 颜色
	element_color.setAttributeNode(colorAttr);

	element_root.appendChild(element_color);

	root.appendChild(element_root);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	qDebug() << "save success";
	return true;
}

bool XmlNodeAction::getFileHandle(QDomDocument& doc, QFile& file, const QSize size)
{
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	if (!QFileInfo::exists(topSvgPath))
	{
		create(topSvgPath, size);
	}
	file.setFileName(topSvgPath);
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		file.close();
		return false;
	}
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return false;
	}
	file.close();
	return true;
}

// 更新节点
bool XmlNodeAction::SLOT_UpdatePoints(const QString nodeId, const QVector<QPointF> points, EditType type)
{
	QString topSvgPath = Config::Instance->GetTopSvgPath();
	QFile file(topSvgPath);
	if (!file.open(QIODevice::ReadOnly | QFile::Text))
	{
		qDebug() << "open for add error...";
		file.close();
		return false;
	}
	QDomDocument doc;
	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		qDebug() << errorStr;
		file.close();
		return false;
	}
	file.close();
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		return false;
	}

	QDomElement userNode = root.firstChildElement();
	std::string rootname = root.nodeName().toStdString();
	bool find = false;
	while (!userNode.isNull() && !find)
	{
		std::string str = userNode.nodeName().toStdString();
		if (str.compare("g") == 0)
		{
			std::string str_c = userNode.attributeNode("id").value().toStdString();
			if (str_c.compare(nodeId.toStdString()) == 0)
			{
				string title;
				if (type == EditType_Polygon)
				{
					title = "polygon";
				}
				QDomElement child = userNode.firstChildElement();
				while (!child.isNull() && !find)
				{
					std::string str = child.nodeName().toStdString();
					if (str.compare(title) == 0)
					{
						QString attr_children;
						for (int index = 0; index < points.count(); ++index)
						{
							QPointF point = points[index];
							attr_children.append(QString::number(point.x()) + "," + QString::number(point.y()));
							if (index < (points.count() - 1))
							{
								attr_children.append(" ");
							}
						}

						QDomElement element_content = doc.createElement(QString::fromStdString(title));
						QDomAttr contentAttr = doc.createAttribute("points");
						contentAttr.setValue(attr_children);
						element_content.setAttributeNode(contentAttr);
						userNode.replaceChild(element_content, child);
						child = element_content.nextSiblingElement();
					}
					else
						child = child.nextSiblingElement();
				}
				find = true;
			}
			userNode = userNode.nextSiblingElement();
		}
	}

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << "open for add error!";
	}
	QTextStream out(&file);
	doc.save(out, 4);
	file.close();
	return true;
}
