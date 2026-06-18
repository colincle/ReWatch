#pragma once

#include <QJsonObject>
#include <QString>

void        ensureDirectoryExists(const QString &path);
void        ensureStorageFileExists(const QString &filePath);
QJsonObject readJsonFile(const QString &filePath);
bool        writeJsonFile(const QString &filePath, const QJsonObject &root);
