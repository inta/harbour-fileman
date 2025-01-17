#include "fileinfo.h"

FileInfo::FileInfo(QObject *parent) :
    QObject(parent)
{
}

/*
 *  Get verbose information about a single file
 */
QVariant FileInfo::getFileInfo(const QString &fullPath)
{
    QFileInfo fileInfo(fullPath);

    QVariantMap replyMap;
    QVariantMap detailEntries;

    QVariantMap fileDetails;

    // Check if it's an image
    QImage image(fullPath);

    // Check if it's a directory
    QDir dir(fullPath);

    bool isDir = dir.exists();

    if (!image.isNull())
    {
        QVariantMap imageDetails;

        // Add details about the image
        imageDetails.insert(tr("Width"), image.width());
        imageDetails.insert(tr("Height"), image.height());

        detailEntries.insert(tr("Image details"), imageDetails);
    }

    if (!isDir)
    {
        // Add normal file details
        fileDetails.insert(tr("Size"), bytesToString(fileInfo.size()));
        fileDetails.insert(tr("Last modified"), fileInfo.lastModified().toString());
        fileDetails.insert(tr("Created"), fileInfo.created().toString());

        detailEntries.insert(tr("File details"), fileDetails);
    }
    else
    {
        // Add directory details
        fileDetails.insert(tr("Last modified"), fileInfo.lastModified().toString());
        fileDetails.insert(tr("Created"), fileInfo.created().toString());

        detailEntries.insert(tr("Directory details"), fileDetails);
    }

    // Get file format name
    QString fileFormat = getFileFormatName(fileInfo.suffix());

    if (fileFormat != "" && !isDir)
        replyMap.insert("fileFormat", fileFormat);
    else
        replyMap.insert("fileFormat", "Directory");

    replyMap.insert("details", detailEntries);

    QVariantMap actionMap = getFileActions(fullPath);

    replyMap.insert("actions", actionMap);

    return QVariant::fromValue(replyMap);
}

/*
 *  Get file content as text
 */
QString FileInfo::getFileContent(const QString &fullPath)
{
    QFile file;

    file.setFileName(fullPath);
    file.open(QIODevice::ReadOnly);

    QString textContent = QString(file.readAll());

    return textContent;
}

/*
 *  Save new file content to a file
 */
void FileInfo::setFileContent(const QString &fullPath, const QString &content)
{
    QFile file;

    file.setFileName(fullPath);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    file.resize(0);
    file.write(content.toLocal8Bit());

    file.flush();
}

QString FileInfo::getFileFormatName(QString suffix)
{
    QMap<QString, QString> formatNames;
    formatNames.insert("png", tr("PNG image"));
    formatNames.insert("jpg", tr("JPEG image"));
    formatNames.insert("jpeg", tr("JPEG image"));
    formatNames.insert("gif", tr("GIF image"));
    formatNames.insert("svg", tr("SVG vector image"));

    formatNames.insert("mpg", tr("MPG video"));
    formatNames.insert("avi", tr("AVI video"));
    formatNames.insert("mov", tr("MOV video"));
    formatNames.insert("3gp", tr("3GP video"));
    formatNames.insert("mp4", tr("MP4 video"));
    formatNames.insert("mkv", tr("MKV video"));
    formatNames.insert("wmv", tr("WMV video"));

    formatNames.insert("mp3", tr("MP3 audio"));
    formatNames.insert("ogg", tr("OGG audio"));
    formatNames.insert("flac", tr("FLAC audio"));

    formatNames.insert("apk", tr("Android application package"));
    formatNames.insert("rpm", tr("RPM package"));

    formatNames.insert("txt", tr("Text file"));

    if (formatNames.contains(suffix))
        return formatNames.value(suffix);
    else
        return tr("unknown");
}

QString FileInfo::bytesToString(qint64 bytes)
{
    QString sign = (bytes < 0 ? "-" : "");
    double readable = (bytes < 0 ? -bytes : bytes);
    QString suffix;
    if (bytes >= 0x1000000000000000) // Exabyte
    {
        suffix = "EiB";
        readable = (bytes >> 50);
    }
    else if (bytes >= 0x4000000000000) // Petabyte
    {
        suffix = "PiB";
        readable = (bytes >> 40);
    }
    else if (bytes >= 0x10000000000) // Terabyte
    {
        suffix = "TiB";
        readable = (bytes >> 30);
    }
    else if (bytes >= 0x40000000) // Gigabyte
    {
        suffix = "GiB";
        readable = (bytes >> 20);
    }
    else if (bytes >= 0x100000) // Megabyte
    {
        suffix = "MiB";
        readable = (bytes >> 10);
    }
    else if (bytes >= 0x400) // Kilobyte
    {
        suffix = "KiB";
        readable = bytes;
    }
    else
    {
        return QString("%1%2 B").arg(sign, QString::number(readable,'f',2)); // Byte
    }

    readable = readable / 1000;

    return QString("%1%2 %3").arg(sign, QString::number(readable,'f',2), suffix);
}

QVariantMap FileInfo::getFileActions(QString fullPath)
{
    QFileInfo fileInfo(fullPath);

    QVariantMap actionMap;

    // If we have a package file, add an action to install it
    if (fileInfo.suffix() == "apk")
    {
        QVariantMap apkAction;
        apkAction.insert("label", tr("Install"));
        apkAction.insert("action", "installApk");
        apkAction.insert("track", true);
        actionMap.insert("Install", apkAction);
    }
    if (fileInfo.suffix() == "rpm")
    {
        QVariantMap rpmAction;
        rpmAction.insert("label", tr("Install"));
        rpmAction.insert("action", "installRpm");
        rpmAction.insert("track", true);
        actionMap.insert("Install", rpmAction);
    }

    // Don't allow users to perform these actions if the file is actually a directory
    if (!fileInfo.isDir())
    {
        QVariantMap textAction;
        textAction.insert("label", tr("Show as text"));
        textAction.insert("action", "showAsText");
        textAction.insert("process", false);
        actionMap.insert("Show as text", textAction);

        QVariantMap editAsTextAction;
        editAsTextAction.insert("label", tr("Edit as text"));
        editAsTextAction.insert("action", "editAsText");
        editAsTextAction.insert("process", false);
        actionMap.insert("Edit as text", editAsTextAction);

        QVariantMap openSystemAction;
        openSystemAction.insert("label", tr("Open"));
        openSystemAction.insert("action", "openSystem");
        openSystemAction.insert("process", true);
        actionMap.insert("Open", openSystemAction);
    }

    if (fileInfo.isExecutable() && !fileInfo.isDir())
    {
        QVariantMap executeAction;
        executeAction.insert("label", tr("Execute"));
        executeAction.insert("action", "execute");
        actionMap.insert("Execute", executeAction);
    }

    return actionMap;
}
