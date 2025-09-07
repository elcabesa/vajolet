#include "../fann/src/include/fann.h"

int main()
{
    const unsigned int num_input = 64*12;
    const unsigned int num_output = 1;

    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 512;
    const float desired_error = (const float) 0.1f;//0.001;
    const unsigned int max_epochs = 10000;
    const unsigned int epochs_between_reports = 1;

    struct fann *ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

    fann_set_activation_function_hidden(ann, FANN_LINEAR_PIECE_RECT_LEAKY);
    fann_set_activation_function_output(ann, FANN_LINEAR);

    fann_train_on_file(ann, "train.data", max_epochs, epochs_between_reports, desired_error);

    fann_save(ann, "eval_float.net");

    fann_destroy(ann);

    return 0;
}
