/***************************************************************************
    File                 : interpolationDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email                : ion_vasilief@yahoo.fr, thzs@gmx.net
    Description          : Interpolation options dialog
                           
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
#include "interpolationDialog.h"
#include "graph.h"
#include "parser.h"
#include "colorBox.h"

#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>

InterpolationDialog::InterpolationDialog( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
		setName( "InterpolationDialog" );
	setWindowTitle(tr("QtiPlot - Interpolation Options"));
	
	QGridLayout *gl1 = new QGridLayout();
	gl1->addWidget(new QLabel(tr("Make curve from")), 0, 0);

	boxName = new QComboBox();
	gl1->addWidget(boxName, 0, 1);
	
	gl1->addWidget(new QLabel(tr("Spline")), 1, 0);
	boxMethod = new QComboBox();
	boxMethod->insertItem(tr("Linear"));
    boxMethod->insertItem(tr("Cubic"));
    boxMethod->insertItem(tr("Non-rounded Akima"));
	gl1->addWidget(boxMethod, 1, 1);
	
	gl1->addWidget(new QLabel(tr("Points")), 2, 0);
	boxPoints = new QSpinBox();
	boxPoints->setRange(3,100000);
	boxPoints->setSingleStep(10);
	boxPoints->setValue(1000);
	gl1->addWidget(boxPoints, 2, 1);

	gl1->addWidget(new QLabel(tr("From Xmin")), 3, 0);
	boxStart = new QLineEdit();
	boxStart->setText(tr("0"));
	gl1->addWidget(boxStart, 3, 1);
	
	gl1->addWidget(new QLabel(tr("To Xmax")), 4, 0);
	boxEnd = new QLineEdit();
	gl1->addWidget(boxEnd, 4, 1);

	gl1->addWidget(new QLabel(tr("Color")), 5, 0);
	
	boxColor = new ColorBox(false);
	boxColor->setColor(QColor(Qt::red));
	gl1->addWidget(boxColor, 5, 1);
	
	QGroupBox *gb1 = new QGroupBox();
    gb1->setLayout(gl1);
	
	buttonFit = new QPushButton(tr( "&Make" ));
    buttonFit->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));
	
	QHBoxLayout *hbox1 = new QHBoxLayout(); 
    hbox1->addWidget(buttonFit);
    hbox1->addWidget(buttonCancel);
    
    QVBoxLayout *vl = new QVBoxLayout();
 	vl->addWidget(gb1);
	vl->addLayout(hbox1);	
	setLayout(vl);
   
    // signals and slots connections
	connect( boxName, SIGNAL( activated(int) ), this, SLOT( activateCurve(int) ) );
	connect( buttonFit, SIGNAL( clicked() ), this, SLOT( interpolate() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

InterpolationDialog::~InterpolationDialog()
{
}

void InterpolationDialog::interpolate()
{
QString curve = boxName->currentText();
QStringList curvesList = graph->curvesList();
if (curvesList.contains(curve) <= 0)
	{
	QMessageBox::critical(this,tr("QtiPlot - Warning"),
		tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curve));
	boxName->clear();
	boxName->insertStringList(curvesList);
	return;
	}

double from, to;
try
	{
	MyParser parser;
	parser.SetExpr(boxStart->text().ascii());
	from=parser.Eval();
	}
catch(mu::ParserError &e)
	{
	QMessageBox::critical(this, tr("QtiPlot - Start limit error"), QString::fromStdString(e.GetMsg()));
	boxStart->setFocus();
	return;
	}		
	
try
	{
	MyParser parser;	
	parser.SetExpr(boxEnd->text().ascii());
	to=parser.Eval();
	}
catch(mu::ParserError &e)
	{
	QMessageBox::critical(this, tr("QtiPlot - End limit error"), QString::fromStdString(e.GetMsg()));
	boxEnd->setFocus();
	return;
	}	

if (from>=to)
	{
	QMessageBox::critical(this, tr("QtiPlot - Input error"),
				tr("Please enter x limits that satisfy: from < to!"));
	boxEnd->setFocus();
	return;
	}
	
int start, end;
int spline = boxMethod->currentItem();
QwtPlotCurve *c = graph->getFitLimits(boxName->currentText(), from, to, spline+3, start, end);
if (!c)
	return;

graph->interpolate(c, spline, start, end, boxPoints->value(), boxColor->currentItem());
}

void InterpolationDialog::setGraph(Graph *g)
{
graph = g;
boxName->insertStringList (g->curvesList(),-1);
	
if (g->selectorsEnabled())
	{
	int index = g->curveIndex(g->selectedCurveID());
	boxName->setCurrentItem(index);
	activateCurve(index);
	}
else
	activateCurve(0);

connect (graph, SIGNAL(closedGraph()), this, SLOT(close()));
connect (graph, SIGNAL(dataRangeChanged()), this, SLOT(changeDataRange()));
};

void InterpolationDialog::activateCurve(int index)
{
QwtPlotCurve *c = graph->curve(index);
if (!c)
	return;

if (graph->selectorsEnabled() && graph->selectedCurveID() == graph->curveKey(index))
	{
	double start = graph->selectedXStartValue();
	double end = graph->selectedXEndValue();
	boxStart->setText(QString::number(QMIN(start, end)));
	boxEnd->setText(QString::number(QMAX(start, end)));
	}
else
	{
	boxStart->setText(QString::number(c->minXValue()));
	boxEnd->setText(QString::number(c->maxXValue()));
	}
};

void InterpolationDialog::changeDataRange()
{
double start = graph->selectedXStartValue();
double end = graph->selectedXEndValue();
boxStart->setText(QString::number(QMIN(start, end), 'g', 15));
boxEnd->setText(QString::number(QMAX(start, end), 'g', 15));
}

