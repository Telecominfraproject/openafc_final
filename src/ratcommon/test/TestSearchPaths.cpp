//

#include "../SearchPaths.h"
#include <memory>

using namespace FileTestHelpers;

namespace
{

void setTestEnv(const QString &locPath, const QString &sysPath)
{
#if defined(Q_OS_LINUX)
	::qputenv("XDG_DATA_DIRS", sysPath.toLocal8Bit());
	::qputenv("XDG_DATA_HOME", locPath.toLocal8Bit());
#elif defined(Q_OS_WIN)
	::qputenv("LOCALAPPDATA",
		  QStringList({locPath, sysPath}).join(QDir::listSeparator()).toLocal8Bit());
#else
	#error Unknown OS define
#endif
}

QStringList filterPathList(const QStringList &list)
{
#if defined(Q_OS_LINUX)
	return list;
#elif defined(Q_OS_WIN)
	// Last four elements are fixed by Qt/OS
	return list.mid(0, list.size() - 4);
#else
	#error Unknown OS define
#endif
}

}

class TestSearchPaths : public testing::Test
{
	protected:
		std::unique_ptr<QCoreApplication> _app;
		std::unique_ptr<FileTestHelpers::TestDir> _tmp;

		/** Construct an absolute path with native separators.
		 * @param subPath The relative path.
		 * @return The corresponding absolute path.
		 */
		QString nativeAbsolutePath(const QString &subPath) const
		{
			if (!_tmp) {
				throw std::logic_error("no tmp dir");
			}
			return QDir::toNativeSeparators(_tmp->absoluteFilePath(subPath));
		}

		static void SetUpTestCase()
		{
			UnitTestHelpers::initLogging();
		}

		virtual void SetUp()
		{
			_app.reset(new UnitTestCoreApp());

			_tmp.reset(new TestDir("paths"));
			ASSERT_TRUE(_tmp->mkdir("system1"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system1/system.txt"), "system1"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system1/only1.txt"), "system1"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system1/read.txt"), "system1"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system1/write.txt"), "system1"));
			ASSERT_TRUE(_tmp->mkdir("system2"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system2/system.txt"), "system2"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system2/only2.txt"), "system2"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system2/read.txt"), "system2"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("system2/write.txt"), "system2"));
			ASSERT_TRUE(_tmp->mkdir("local"));
			ASSERT_TRUE(makeFile(nativeAbsolutePath("local/read.txt"), "local"));
		}
		virtual void TearDown()
		{
			_tmp.reset();
			_app.reset();
		}
};

TEST_F(TestSearchPaths, forReadingSystem)
{
	const QString sysPath = nativeAbsolutePath("system1");
	const QString locPath = nativeAbsolutePath("missing");

	setTestEnv(locPath, sysPath);
	ASSERT_TRUE(SearchPaths::init(QString()));

	// Only one path
	ASSERT_EQ(filterPathList(QDir::searchPaths("data")), QStringList({locPath, sysPath}));

	ASSERT_EQ(content("data:system.txt"), QByteArray("system1"));
	ASSERT_EQ(content("data:read.txt"), QByteArray("system1"));
}

TEST_F(TestSearchPaths, forReadingSearchOrder)
{
	const QString sys1Path = nativeAbsolutePath("system1");
	const QString sys2Path = nativeAbsolutePath("system2");
	const QString locPath = nativeAbsolutePath("missing");

	// System-2 gets priority
	setTestEnv(locPath, QStringList({sys2Path, sys1Path}).join(QDir::listSeparator()));
	ASSERT_TRUE(SearchPaths::init(QString()));

	// Priority order
	ASSERT_EQ(filterPathList(QDir::searchPaths("data")),
		  QStringList({locPath, sys2Path, sys1Path}));

	ASSERT_EQ(content("data:only1.txt"), QByteArray("system1"));
	ASSERT_EQ(content("data:only2.txt"), QByteArray("system2"));
	ASSERT_EQ(content("data:system.txt"), QByteArray("system2"));
	ASSERT_EQ(content("data:read.txt"), QByteArray("system2"));
}

TEST_F(TestSearchPaths, forReadingLocal)
{
	const QString sysPath = nativeAbsolutePath("system1");
	const QString locPath = nativeAbsolutePath("local");

	setTestEnv(locPath, sysPath);
	ASSERT_TRUE(SearchPaths::init(QString()));

	// Local gets priority
	ASSERT_EQ(filterPathList(QDir::searchPaths("data")), QStringList({locPath, sysPath}));

	ASSERT_EQ(content("data:system.txt"), QByteArray("system1"));
	ASSERT_EQ(content("data:read.txt"), QByteArray("local"));
}

TEST_F(TestSearchPaths, forWritingLocal)
{
	const QString sysPath = nativeAbsolutePath("system1");
	const QString locPath = nativeAbsolutePath("local");

	setTestEnv(locPath, sysPath);
	ASSERT_TRUE(SearchPaths::init(QString()));

	// Local gets priority
	ASSERT_EQ(filterPathList(QDir::searchPaths("data")), QStringList({locPath, sysPath}));

	// When reading the file, the system is selected first
	ASSERT_EQ(content("data:write.txt"), QByteArray("system1"));
	// Writing based on search path opens the system file
	{
		QFile file("data:write.txt");
		file.open(QFile::WriteOnly);
		// non-native separators generated by Qt
		ASSERT_EQ(file.fileName(), _tmp->absoluteFilePath("system1/write.txt"));
	}
	// Must use custom function to open the correct path
	{
		QFile file(SearchPaths::forWriting("data", "write.txt"));
		ASSERT_TRUE(file.open(QFile::WriteOnly));
		ASSERT_EQ(file.fileName(), _tmp->absoluteFilePath("local/write.txt"));
		ASSERT_GT(file.write("newdata"), 0);
	}
	ASSERT_EQ(content("data:write.txt"), QByteArray("newdata"));
}

TEST_F(TestSearchPaths, pathSuffix)
{
	const QString sysPath = nativeAbsolutePath("system1");
	const QString locPath = nativeAbsolutePath("local");

	setTestEnv(locPath, sysPath);
	ASSERT_TRUE(SearchPaths::init("suffix"));

	// Local gets priority
	ASSERT_EQ(filterPathList(QDir::searchPaths("data")),
		  QStringList({
			  (locPath + QDir::separator() + "suffix"),
			  (sysPath + QDir::separator() + "suffix"),
		  }));
}
