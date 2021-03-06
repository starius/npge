/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BLOCKSETWIDGET_HPP
#define BLOCKSETWIDGET_HPP

#include <QtGui>

#ifndef Q_MOC_RUN
#include "gui-global.hpp"
#include "global.hpp"
#endif

using namespace npge;

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

    BlockSetPtr block_set() const;

    void set_genes(BlockSetPtr genes);

    void set_split_parts(BlockSetPtr split_parts);

    void set_low_similarity(BlockSetPtr low_similarity);

    void set_bsa(std::string bsa_name);

    static void moveBsaWidget(BlockSetWidget* dst,
                              BlockSetWidget* src);

signals:
    void blockClicked(QString name);
    void bsaFragmentClicked(Fragment* fragment);

private slots:
    void onblockClicked(QString name);

private:
    Ui::BlockSetWidget* ui;
    AlignmentView* alignment_view_;
    AlignmentModel* alignment_model_;
    BlockSetModel* block_set_model_;
    QSortFilterProxyModel* proxy_model_;
    BSAModel* bsa_model_;
    BSAView* bsa_view_;
    int prev_row_;
    std::map<const Block*, Fragments> fragments_;
    typedef std::map<Fragment*, Fragment*> F2F;
    F2F normal2global_;

private slots:
    void set_block(const Block* block);
    void clicked_f(const QModelIndex& index);
    void bsa_clicked(const QModelIndex& index);
    void jump_to_f(Fragment* fragment, int col);
    void fragment_selected_f(Fragment* fragment);

    void on_nonunique_stateChanged(int state);
    void on_bsaComboBox_activated(QString bsa_name);

    void update_gene_layout();

    void alignment_clicked(const QModelIndex& index);

    void on_blockNameLineEdit_editingFinished();
    void on_clearBlockNameButton_clicked();

    void on_actionCopy_block_name_triggered();
    void on_actionCopy_fragment_id_triggered();

    void onSearchStarted();
    void onSearchFinished();
};

#endif // BLOCKSETWIDGET_HPP

