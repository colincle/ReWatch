#pragma once

#include "Title.hpp"

#include <vector>
#include <QString>

class AppUtils
{
public:
    AppUtils();
    void setOmdbApiKey(QString key);

    QString getKey() {return omdbApiKey;}

    private:
    QString appFilePath;
    QString omdbApiKey;
    std::vector<Title> titles;

    void load();
};