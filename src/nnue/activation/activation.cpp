#include "activation.h"
#include "relu.h"
#include "linear.h"

Activation::Activation() {}

Activation::~Activation() {}

std::unique_ptr<Activation> ActivationFactory::create( const ActivationFactory::type t )
{
	if( t == type::linear)
	{
		return std::make_unique<linearActivation>();
	}
	else if(t == type::relu)
	{
		return std::make_unique<reluActivation>();
	}
	return std::make_unique<linearActivation>();
}

