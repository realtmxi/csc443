
#include "lsm_tree.h"

#include "include/common/config.h"
#include "include/storage/page/b_tree_internal_page.h"
#include "include/storage/page/b_tree_leaf_page.h"
#include "include/storage/page/b_tree_page.h"

LSMTree::LSMTree()
{
    level_ = 0;
    filename_ = "";
}

std::string
LSMTree::GetFilename() const
{
    return filename_;
}

bool
LSMTree::TestAndSetLevelAndFIlename(const std::string &btree1_filename,
                                    const std::string &btree2_filename)
{
    std::string level1 =
        btree1_filename.substr(4, btree1_filename.find('_', 4) - 4);
    std::string level2 =
        btree2_filename.substr(4, btree2_filename.find('_', 4) - 4);

    if (level1 != level2)
    {
        printf("Cannot merge BTree files of different levels\n");
        return false;
    }

    level_ = std::stoi(level1) + 1;
    filename_ = "sst_" + std::to_string(level_) + "_" +
                std::to_string(std::time(nullptr)) + ".sst";
    return true;
}

BTreePage *
LSMTree::ReadPage(std::ifstream &input)
{
    IndexPageType page_type;
    input.read(reinterpret_cast<char *>(&page_type), sizeof(IndexPageType));

    if (page_type == IndexPageType::LEAF_PAGE)
    {
        // TODO: build leaf page object
    }

    if (page_type == IndexPageType::INTERNAL_PAGE)
    {
        // TDOD: build internal page object
    }

    return nullptr;
}

void
LSMTree::WritePage(std::ofstream &output, BTreePage *page)
{
    IndexPageType page_type = page->IsLeafPage() ? IndexPageType::LEAF_PAGE
                                                 : IndexPageType::INTERNAL_PAGE;
    output.write(reinterpret_cast<char *>(&page_type), sizeof(IndexPageType));

    if (page_type == IndexPageType::LEAF_PAGE)
    {
        // serialize leaf page. likely can use existing serialization code
    }

    if (page_type == IndexPageType::INTERNAL_PAGE)
    {
        // serialize internal page. likely can use existing serialization code
    }
}
