#include "../fann/src/include/floatfann.h"
#include <string>
int FANN_API test_callback(struct fann *ann, struct fann_train_data *train,
                           unsigned int max_epochs, unsigned int epochs_between_reports,
                           float desired_error, unsigned int epochs)
{
    printf("Epochs     %8d. MSE: %.5f. Desired-MSE: %.5f\n", epochs, fann_get_MSE(ann), desired_error);
    //if(epochs %10 == 0)
    {
        printf("SAVE ANN %i\n",epochs);
        std::string name ="eval_float";
        name+=std::to_string(epochs);
        name+=".net";
        fann_save(ann, name.c_str());
    }
    return 0;
}

int main()
{
    const unsigned int num_input = 64*12;
    const unsigned int num_output = 1;

    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 512;
    const float desired_error = (const float) 0.01f;//0.001;
    const unsigned int max_epochs = 10000;
    const unsigned int epochs_between_reports = 1;

    struct fann *ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

    fann_set_callback(ann, &test_callback);

    fann_set_activation_function_hidden(ann, FANN_LINEAR_PIECE_RECT_LEAKY);
    fann_set_activation_function_output(ann, FANN_LINEAR);

    fann_train_on_file(ann, "train.data", max_epochs, epochs_between_reports, desired_error);

    fann_save(ann, "eval_float.net");

    fann_destroy(ann);

    return 0;
}
