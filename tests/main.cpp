#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTest>

#include <cstdio>
#include <unistd.h>

extern QObject *createTestAppStorage();
extern QObject *createTestImportedFileValidator();
extern QObject *createTestTitleSearch();
extern QObject *createTestOmdbSearch();
extern QObject *createTestLibraryUpdate();
extern QObject *createTestLibraryUpdateController();
extern QObject *createTestAddBar();
extern QObject *createTestAppMenuBar();
extern QObject *createTestLibraryViewTopBar();
extern QObject *createTestTopBar();
extern QObject *createTestHoverButton();
extern QObject *createTestIconButton();
extern QObject *createTestIconTextButton();
extern QObject *createTestStreamingPlatformButton();
extern QObject *createTestTextButton();
extern QObject *createTestAddStreamingPlatformDialog();
extern QObject *createTestElidedLabel();
extern QObject *createTestErrorCard();
extern QObject *createTestFlowWidget();
extern QObject *createTestSearchBar();
extern QObject *createTestHelpDialog();
extern QObject *createTestLibraryView();
extern QObject *createTestMainWindow();
extern QObject *createTestRankingView();
extern QObject *createTestTitleCard();
extern QObject *createTestSettingsWindow();
extern QObject *createTestSearchResults();
extern QObject *createTestTitleDetailView();
extern QObject *createTestJsonFileIO();
extern QObject *createTestLevenshtein();
extern QObject *createTestSvgUtils();

static constexpr const char *LOG_FILE = "test_results.log";

static int runSuite(QObject *obj, int argc, char *argv[], QTextStream &log)
{
	// Redirect stdout to a temp file so we can filter it
	int   savedFd = dup(STDOUT_FILENO);
	FILE *tmp = tmpfile();
	dup2(fileno(tmp), STDOUT_FILENO);

	int result = QTest::qExec(obj, argc, argv);
	delete obj;

	fflush(stdout);
	dup2(savedFd, STDOUT_FILENO);
	close(savedFd);

	// Read captured output, filter banners for console, extract failures for log
	rewind(tmp);
	char    buf[4096];
	QString captured;
	while(fgets(buf, sizeof(buf), tmp))
		captured += buf;
	fclose(tmp);

	// Known-benign Qt warnings emitted by passing tests: the deliberately-invalid
	// OMDb key tests (server rejects the bad key) and the button tests that pass an
	// empty icon path on purpose. These are not bugs or failures, so drop them from
	// both console and log. Any other warning still surfaces.
	auto isBenignWarning = [](const QString &line)
	{
		return line.startsWith("QWARN") &&
		       (line.contains("qt.network.http2") ||
		        line.contains("Host requires authentication") ||
		        line.contains("QFSFileEngine::open: No file name specified") ||
		        line.contains("Failed to open file for reading") ||
		        line.contains("Failed to open file for writing"));
	};

	bool inFailure = false;
	for(const QString &line : captured.split('\n'))
	{
		if(isBenignWarning(line))
			continue;

		const bool isBanner = line.startsWith("*****") || line.startsWith("Config:");
		if(!isBanner)
			printf("%s\n", qPrintable(line));

		if(line.startsWith("FAIL!") || line.startsWith("QFATAL"))
		{
			inFailure = true;
			log << line << "\n";
		}
		else if(line.startsWith("QWARN"))
		{
			// Warnings are standalone; log them without entering failure mode.
			log << line << "\n";
		}
		else if(inFailure && (line.startsWith("   ") || line.startsWith("\t")))
		{
			log << line << "\n";
		}
		else
		{
			inFailure = false;
		}
	}

	return result;
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	printf("OMDB API key (press Enter to skip network tests): ");
	fflush(stdout);
	char keyBuf[256] = {};
	if(fgets(keyBuf, sizeof(keyBuf), stdin))
	{
		QString key = QString(keyBuf).trimmed();
		if(!key.isEmpty())
			qputenv("OMDB_API_KEY", key.toUtf8());
	}

	QFile logFile(LOG_FILE);
	(void)logFile.open(QFile::WriteOnly | QFile::Truncate);
	QTextStream log(&logFile);

	const QList<QObject *> suites = {
	    createTestAppStorage(),
	    createTestImportedFileValidator(),
	    createTestTitleSearch(),
	    createTestOmdbSearch(),
	    createTestLibraryUpdate(),
	    createTestLibraryUpdateController(),
	    createTestAddBar(),
	    createTestAppMenuBar(),
	    createTestLibraryViewTopBar(),
	    createTestTopBar(),
	    createTestHoverButton(),
	    createTestIconButton(),
	    createTestIconTextButton(),
	    createTestStreamingPlatformButton(),
	    createTestTextButton(),
	    createTestAddStreamingPlatformDialog(),
	    createTestElidedLabel(),
	    createTestErrorCard(),
	    createTestFlowWidget(),
	    createTestSearchBar(),
	    createTestHelpDialog(),
	    createTestLibraryView(),
	    createTestMainWindow(),
	    createTestRankingView(),
	    createTestTitleCard(),
	    createTestSettingsWindow(),
	    createTestSearchResults(),
	    createTestTitleDetailView(),
	    createTestJsonFileIO(),
	    createTestLevenshtein(),
	    createTestSvgUtils(),
	};

	int passed = 0, failed = 0;
	for(QObject *suite : suites)
	{
		int result = runSuite(suite, argc, argv, log);
		result == 0 ? passed++ : failed++;
		printf("\n");
	}

	logFile.close();

	if(failed == 0)
		printf("\033[32m  %d/%d suites passed\033[0m\n", passed, passed + failed);
	else
		printf(
		    "\033[31m  %d/%d suites failed — see build/%s\033[0m\n",
		    failed,
		    passed + failed,
		    LOG_FILE
		);

	return failed > 0 ? 1 : 0;
}
