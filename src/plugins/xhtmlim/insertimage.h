#ifndef INSERTIMAGE_H
#define INSERTIMAGE_H

#include <QUrl>
#include <QNetworkAccessManager>
#include <QBuffer>
#include <interfaces/ibitsofbinary.h>

#include "ui_insertimage.h"

class XhtmlIm;

namespace Ui {
    class InsertImage;
}

class InsertImage : public QDialog
{
    Q_OBJECT

public:
	InsertImage(XhtmlIm *AXhtmlIm, QNetworkAccessManager *ANetworkAccessManager, const QByteArray &AImageData, const QUrl &AImageUrl, const QSize &AImageSize, const QString &AAlternativeText, QWidget *parent = 0);
    ~InsertImage();
    Ui::InsertImage *ui;

    const QUrl       &getUrl() const {return FUrlCurrent;}
    const QString    &getFileType() const {return FMimeType;}
    const QByteArray &getImageData() const {return FImageData;}
    const QByteArray &getOriginalFormat() const {return FOriginalFormat;}
    const QByteArray getSelectedFormat() const {return ui->cmbType->itemData(ui->cmbType->currentIndex()).toByteArray();}
    const QString    getAlternativeText() const {return ui->ledAlt->text();}

    int originalWidth() const {return FSizeOld.width();}
    int originalHeight() const {return FSizeOld.height();}
    int newWidth() const {return ui->spbWidth->value();}
    int newHeight() const {return ui->spbHeight->value();}
    int physResize() const {return ui->chbPhysResize->isChecked() &&
                                  (FSizeOld.width()!=ui->spbWidth->value() ||
                                   FSizeOld.height()!=ui->spbHeight->value());}
    int getMaxAge() const;
    bool embed() const {return ui->chbEmbed->isChecked();}
    bool isRemote() const;
    static void setBob(IBitsOfBinary *ABitsOfBinary) {FBitsOfBinary = ABitsOfBinary;}

protected:
    bool runMovie(QIODevice *ASourceDevice);
    void startLoadFile(const QUrl &AUrl);
    void disableCommon(bool ADisable=true);
    void disableBOB(bool ADisable=true);
    void readImageData(const QUrl &AUrl);
    void calculateUrl(const QByteArray &AImageData);
	void updateInfoLine(qint64 AImageSize, const QByteArray &AImageFormat, int AWidth, int AHeight);
    void recalculateImageData();
    void enableInsert();
	void updateSpinboxPixels(QSpinBox *ASpinBox);

protected slots:
    void onButtonLoad();
    void onButtonBrowse();
	void onSchemeChanged(int AIndex);
    void onLoadFinished();
    void onTextChanged();
	void onCheckBoxKeepAspect(int AState);
	void onCheckBoxPhysResize(int AState);
	void onSpbWidth(int AWidth);
	void onSpbHeight(int AHeight);
	void onDSpbWidth(double AWidth);
	void onDSpbHeight(double AHeight);
    void onImageSettingsChanged();
	void onMaxAgeChanged(int AValue);

private:
    QString FMimeType;
    QSize   FSizeOld;

    QImage  FImageOriginal;


    QSize   FSizeCurrent;
    QByteArray FFormatCurrent;
    QUrl       FUrlCurrent;

//    QByteArray FOriginalImageData;
	QBuffer		FOriginalImageData;
	QByteArray	FImageData;
	QByteArray	FOriginalFormat;

	QNetworkAccessManager	*FNetworkAccessManager;
	XhtmlIm					*FXhtmlIm;

    QList<QByteArray> FWriterFormats;
    QList<QByteArray> FReaderFormats;
    const QStringList FSchemeMasks;
    bool    FIgnoreSpbW;
    bool    FIgnoreSpbH;
    bool    FIgnoreDspbW;
    bool    FIgnoreDspbH;
	int		FFormatOffset;
    static  IBitsOfBinary *FBitsOfBinary;
};

#endif // INSERTIMAGE_H
