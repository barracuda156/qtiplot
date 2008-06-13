/***************************************************************************
    File                 : TexWidget.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : A widget displaying Tex content as image in 2D plots

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "TexWidget.h"
#include <QPainter>
#include <QBuffer>
#include <QMessageBox>

TexWidget::TexWidget(Graph *plot, const QString& s, const QPixmap& pix):FrameWidget(plot),
d_pix(pix),
d_formula(s),
d_margin(5)
{
	if (!d_pix.isNull())
		setSize(QSize(pix.width() + 2*d_margin, pix.height() + 2*d_margin));
}
	
void TexWidget::paintEvent(QPaintEvent *e)
{
	if (d_pix.isNull())
		return;
	
	QPainter p(this);	
	drawFrame(&p, rect());
	
	QRect pixRect = QRect (d_margin, d_margin, d_pix.width(), d_pix.height());
	p.drawPixmap(pixRect, d_pix);
	
	e->accept();
}

void TexWidget::print(QPainter *painter, const QwtScaleMap map[QwtPlot::axisCnt])
{
	int x = map[QwtPlot::xBottom].transform(calculateXValue());
	int y = map[QwtPlot::yLeft].transform(calculateYValue());

	QRect rect = QRect(x, y, width(), height());
	drawFrame(painter, rect);
	
	QRect pixRect = QRect (x + d_margin, y + d_margin, d_pix.width(), d_pix.height());
	painter->drawPixmap(pixRect, d_pix);
}

void TexWidget::setPixmap(const QPixmap& pix)
{
	d_pix = pix;
	int width = pix.width() + 2*d_margin;
	int height = pix.height() + 2*d_margin;
	if (d_frame == Shadow){
		width += d_margin;
		height += d_margin;
	}
	setSize(QSize(width, height));
	repaint();
}

void TexWidget::clone(TexWidget* t)
{
	d_frame = t->frameStyle();
	d_color = t->frameColor();
	d_formula = t->formula();
	d_pix = t->pixmap();
	resize(t->size());
	setOriginCoord(t->xValue(), t->yValue());
	d_x_right = t->right();
	d_y_bottom = t->bottom();
}

QString TexWidget::saveToString()
{
	QString s = "<TexFormula>\n";
	s += FrameWidget::saveToString();
	s += "<tex>" + d_formula + "</tex>\n";
	s += "<xpm>\n";
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	d_pix.save(&buffer, "XPM");
	s += QString(bytes);
	s += "</xpm>\n";
	return s + "</TexFormula>\n";
}

void TexWidget::restore(Graph *g, const QStringList& lst)
{	
	int frameStyle = 0;
	QColor frameColor = Qt::black;
	double x = 0.0, y = 0.0, right = 0.0, bottom = 0.0;
	QPixmap pix;
	QStringList::const_iterator line;
	QString formula;
	for (line = lst.begin(); line != lst.end(); line++){
        QString s = *line;
        if (s.contains("<Frame>"))
			frameStyle = s.remove("<Frame>").remove("</Frame>").toInt();
		else if (s.contains("<Color>"))
			frameColor = QColor(s.remove("<Color>").remove("</Color>"));
		else if (s.contains("<x>"))
			x = s.remove("<x>").remove("</x>").toDouble();
		else if (s.contains("<y>"))
			y = s.remove("<y>").remove("</y>").toDouble();
		else if (s.contains("<right>"))
			right = s.remove("<right>").remove("</right>").toDouble();
		else if (s.contains("<bottom>"))
			bottom = s.remove("<bottom>").remove("</bottom>").toDouble();
		else if (s.contains("<tex>"))
			formula = s.remove("<tex>").remove("</tex>");
		else if (s.contains("<xpm>")){
			QString xpm;
			while ( s != "</xpm>" ){
				s = *(++line);
				xpm += s + "\n";
			}
			QImage image;
    		if (image.loadFromData(xpm.toAscii()))
				pix = QPixmap::fromImage(image);
		}
	}
	TexWidget *t = g->addTexFormula(formula, pix);
	t->setFrameStyle(frameStyle);
	t->setFrameColor(frameColor);
	t->setCoordinates(x, y, right, bottom);
}
