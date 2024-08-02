#ifndef STYLEDITEMDELEGATE_H
#define STYLEDITEMDELEGATE_H

#include <QComboBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTextDocument>

class StyledItemDelegate : public QStyledItemDelegate {
public:
    StyledItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {
        QComboBox *comboBox = qobject_cast<QComboBox *>(parent);
        // on combobox change item...
        if (comboBox) {
            connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
                QTextDocument doc;
                doc.setHtml(comboBox->itemData(index).toString());
                comboBox->setItemText(index, doc.toPlainText());
            });
        }
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();

        // Definir o texto do item com HTML
        QString html = index.data().toString();
        QTextDocument doc;
        doc.setHtml(html);

        // Configurar a posição e a largura do texto
        painter->translate(option.rect.topLeft());
        doc.setTextWidth(option.rect.width());
        doc.drawContents(painter);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // Definir o tamanho do item
        QString html = index.data().toString();
        QTextDocument doc;
        doc.setHtml(html);
        doc.setTextWidth(option.rect.width());
        return QSize(doc.idealWidth(), doc.size().height());
    }
};

#endif // STYLEDITEMDELEGATE_H
