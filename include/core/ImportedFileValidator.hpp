// Guards against malicious zip imports — validates file type and checks for path-traversal
// entries before extraction.
#pragma once

#include <QString>

struct ValidationResult
{
	bool    valid = false;
	QString error;
};

class ImportedFileValidator
{
  public:
	static ValidationResult validate(const QString &filePath);
	static bool             entriesAreSafe(const QString &zipPath);
};
