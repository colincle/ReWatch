#include "JsonFileIO.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QMessageBox>

void ensureDirectoryExists(const QString &path)
{
	QDir dir(path);

	if(!dir.exists())
	{
		dir.mkpath(path);
	}
}

void ensureStorageFileExists(const QString &filePath)
{
	QFile file(filePath);

	if(file.exists())
		return;

	if(!file.open(QIODevice::WriteOnly))
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Storage Error");
		msgBox.setText("Could not create storage file:\n" + filePath);
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
		msgBox.exec();
		exit(1);
	}
}

QJsonObject readJsonFile(const QString &filePath)
{
	QFile file(filePath);

	if(!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to open file for reading:" << filePath;
		return {};
	}

	QByteArray data = file.readAll();
	file.close();

	return QJsonDocument::fromJson(data).object();
}

bool writeJsonFile(const QString &filePath, const QJsonObject &root)
{
	QFile file(filePath);

	if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qWarning() << "Failed to open file for writing:" << filePath;
		return false;
	}

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();
	return true;
}
