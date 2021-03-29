
#include <mlpack/core.hpp>
#include <mlpack/methods/random_forest/random_forest.hpp>
#include <mlpack/methods/decision_tree/random_dimension_select.hpp>

using namespace mlpack::tree;

int main(int argc, char * argv[])
{
    if (argc != 3)
        return EXIT_FAILURE;

    // load the dataset
    arma::mat dataset;
    bool loaded = mlpack::data::Load(argv[1], dataset);
    if (!loaded)
        return EXIT_FAILURE;

    // extract the labels
    arma::Row<size_t> labels;
    labels = arma::conv_to<arma::Row<size_t>>::from(dataset.row(dataset.n_rows - 1));
    dataset.shed_row(dataset.n_rows - 1);

    // parameters
    /* TODO: read w/ cnpy <29-03-21, cneyton> */
    const size_t n_classes = 2;
    const size_t n_trees   = 10;
    const size_t min_leaf_size = 5;

    auto rf = RandomForest<GiniGain, RandomDimensionSelect>(dataset, labels,
                                                            n_classes, n_trees, min_leaf_size);

    arma::Row<size_t> predictions;
    rf.Classify(dataset, predictions);
    const size_t correct = arma::accu(predictions == labels);
    std::cout << "\nTraining Accuracy: " << (double(correct) / double(labels.n_elem));

    // save  the model
    mlpack::data::Save("qi_model.xml", "qi_model", rf, true);
}
