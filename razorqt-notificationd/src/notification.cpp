/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QtGui/QPainter>
#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtDBus/QDBusArgument>

#include <qtxdg/xdgicon.h>

#include "notification.h"
#include "notificationwidgets.h"

#include <QtDebug>


#define ICONSIZE QSize(32, 32)


Notification::Notification(const QString &application,
                           const QString &summary, const QString &body,
                           const QString &icon, int timeout,
                           const QStringList& actions, const QVariantMap& hints,
                           QWidget *parent)
    : QWidget(parent),
      m_timer(0),
      m_actionWidget(0)
{
    setupUi(this);
    setObjectName("Notification");
    
    setMaximumWidth(parent->width());
    setMinimumWidth(parent->width());

    setValues(application, summary, body, icon, timeout, actions, hints);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeButton_clicked()));
}

void Notification::setValues(const QString &application,
                             const QString &summary, const QString &body,
                             const QString &icon, int timeout,
                             const QStringList& actions, const QVariantMap& hints)
{
    // Basic properties *********************

    // Notifications spec set real order here:
    // An implementation which only displays one image or icon must
    // choose which one to display using the following order:
    //  - "image-data"
    //  - "image-path"
    //  - app_icon parameter
    //  - for compatibility reason, "icon_data"
    if (!hints["image_data"].isNull())
    {
        m_pixmap = getPixmapFromHint(hints["image_data"]);
//        qDebug() << application << "from image_data" << m_pixmap.isNull();
    }
    else if (!hints["image_path"].isNull())
    {
        m_pixmap = getPixmapFromString(hints["image_path"].toString());
//        qDebug() << application << "from image_path" << m_pixmap.isNull();
    }
    else if (!icon.isEmpty())
    {
        m_pixmap = getPixmapFromString(icon);
//        qDebug() << application << "from icon" << icon << m_pixmap.isNull();
    }
    else if (!hints["icon_data"].isNull())
    {
       m_pixmap = getPixmapFromHint(hints["icon_data"]);
//       qDebug() << application << "from icon_data" << m_pixmap.isNull();
    }
    // failback
    if (m_pixmap.isNull())
    {
//        qDebug() << "An icon for name:" << icon << "or hints" << hints << "not found. Using failback.";
        m_pixmap = XdgIcon::defaultApplicationIcon().pixmap(ICONSIZE);
    }

    if (m_pixmap.size().width() > ICONSIZE.width()
    	|| m_pixmap.size().height() > ICONSIZE.height())
    {
    	m_pixmap = m_pixmap.scaled(ICONSIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    iconLabel->setPixmap(m_pixmap);

    // application
	appLabel->setVisible(!application.isEmpty());
    appLabel->setText(application);

    // summary
    summaryLabel->setVisible(!summary.isEmpty());
    summaryLabel->setText(summary);

    // body
    bodyLabel->setVisible(!body.isEmpty());
    bodyLabel->setText(body);

    // Timeout
    // Special values:
    //  < 0: server decides timeout
    //    0: infifite
    if (m_timer)
    {
        m_timer->stop();
        m_timer->deleteLater();
    }

    // -1 for server decides is handled in notifyd to save QSettings instance
    if (timeout > 0)
    {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SIGNAL(timeout()));
        m_timer->start(timeout);
    }

    // Categories *********************
    // TODO/FIXME: Categories - how to handle it?
    if (!hints["category"].isNull())
    {
        qDebug() << "Notification" << application << "category" << hints["category"];
    }

    // Urgency Levels *********************
    // Type    Description
    // 0   Low
    // 1   Normal
    // 2   Critical
    // TODO/FIXME: Urgencies - how to handle it?
    if (!hints["urgency"].isNull())
    {
        qDebug() << "Notification" << application << "urgency" << hints["urgency"];
    }

    // Actions
    if (actions.count() && m_actionWidget == 0)
    {
        m_actionWidget = new NotificationActionsWidget(actions, this);
        connect(m_actionWidget, SIGNAL(actionTriggered(const QString &)),
                this, SIGNAL(actionTriggered(const QString &)));
        actionsLayout->addWidget(m_actionWidget);
        m_actionWidget->show();
    }

    adjustSize();
    // ensure layout expansion
    setMinimumHeight(qMax(rect().height(), childrenRect().height()));
}

void Notification::closeButton_clicked()
{
    if (m_timer)
        m_timer->stop();
    emit userCanceled();
}

void Notification::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QPixmap Notification::getPixmapFromHint(const QVariant &argument) const
{
    int width, height, rowstride, bitsPerSample, channels;
    bool hasAlpha;
    QByteArray data;

    const QDBusArgument arg = argument.value<QDBusArgument>();
    arg.beginStructure();
    arg >> width;
    arg >> height;
    arg >> rowstride;
    arg >> hasAlpha;
    arg >> bitsPerSample;
    arg >> channels;
    arg >> data;
    arg.endStructure();
    QImage img = QImage((uchar*)data.constData(), width, height, QImage::Format_ARGB32).rgbSwapped();

    return QPixmap::fromImage(img);
}

QPixmap Notification::getPixmapFromString(const QString &str) const
{
    QUrl url(str);
    if (url.isValid() && QFile::exists(url.toLocalFile()))
    {
//        qDebug() << "    getPixmapFromString by URL" << url;
        return QPixmap(url.toLocalFile());
    }
    else
    {
//        qDebug() << "    getPixmapFromString by XdgIcon theme" << str << ICONSIZE << XdgIcon::themeName();
//        qDebug() << "       " << XdgIcon::fromTheme(str) << "isnull:" << XdgIcon::fromTheme(str).isNull();
        return XdgIcon::fromTheme(str, XdgIcon::defaultApplicationIcon()).pixmap(ICONSIZE);
    }
}