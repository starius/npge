#ifndef BLOCKSETWIDGET_HPP
#define BLOCKSETWIDGET_HPP

#include <QtGui>

#include "gui-global.hpp"
#include "global.hpp"

using namespace bloomrepeats;

namespace Ui {
class BlockSetWidget;
}

class BlockSetModel;

class BSAModel;
class BSAView;

class BlockSetWidget : public QWidget {
    Q_OBJECT

public:
    explicit BlockSetWidget(BlockSetPtr block_set = BlockSetPtr(),
                            QWidget* parent = 0);
    ~BlockSetWidget();

    void set_block_set(BlockSetPtr block_set);

    void set_genes(BlockSetPtr genes);

    void set_bsa(const std::string& bsa_name);

private:
    Ui::BlockSetWidget* ui;
    AlignmentView* alignment_view_;
    AlignmentModel* alignment_model_;
    BlockSetModel* block_set_model_;
    QSortFilterProxyModel* proxy_model_;
    BSAModel* bsa_model_;
    BSAView* bsa_view_;
    int prev_row_;
    std::map<const Block*, std::vector<const Fragment*> > fragments_;

private slots:
    void set_block(const Block* block);
    void clicked_f(const QModelIndex& index);
    void bsa_clicked(const QModelIndex& index);
    void jump_to_f(Fragment* fragment, int col);
    void fragment_selected_f(Fragment* fragment, int col);

    void on_nonunique_stateChanged(int state);
    void on_bsaComboBox_activated(QString bsa_name);

    void update_gene_layout();

    void alignment_clicked(const QModelIndex& index);

    void on_blockNameLineEdit_editingFinished();

    void on_actionCopy_block_name_triggered();
    void on_actionCopy_fragment_id_triggered();
};

#endif // BLOCKSETWIDGET_HPP

