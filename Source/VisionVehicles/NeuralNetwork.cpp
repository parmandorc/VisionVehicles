// Fill out your copyright notice in the Description page of Project Settings.

#include "VisionVehicles.h"
#include "NeuralNetwork.h"


UNeuralNetwork::UNeuralNetwork()
{

}

UNeuralNetwork* UNeuralNetwork::GetInstance()
{
	return NewObject<UNeuralNetwork>();
}

void UNeuralNetwork::Init(int inputs, int outputs, TArray<int> hiddenLayers, 
	float _initialLearningRate, float _learningRateDecay)
{
	// Set dimensions
	nInputs = inputs;
	nOutputs = outputs;
	nHiddenLayers = hiddenLayers;

	TArray<int> dimensions;
	dimensions.Add(nInputs);
	dimensions.Append(nHiddenLayers);
	dimensions.Add(nOutputs);

	// Initialize weights
	weights.Empty();
	for (int l = 1; l < dimensions.Num(); l++)
	{
		TArray<TArray<float>> layer; // The weights for the current layer

		for (int j = 0; j < dimensions[l]; j++)
		{
			TArray<float> w; // The weights for the current unit
			for (int i = 0; i < dimensions[l - 1] + 1; i++) // +1: include the weight for the bias
			{
				w.Add(FMath::RandRange(-1.0f, 1.0f));
			}
			layer.Add(w);
		}

		weights.Add(layer);
	}

	// Set learning rate
	epoche = 0;
	initialLearningRate = _initialLearningRate;
	learningRateDecay = _learningRateDecay;
	learningRate = initialLearningRate;
}

TArray<float> UNeuralNetwork::Run(TArray<float> inputs, TArray<TArray<float>>* weightedSums, TArray<TArray<float>>* activations)
{
	weightedSums->Empty();
	activations->Empty();

	// Initialize the first layer with the input values
	TArray<float> inputsActivations;
	for (float value : inputs) inputsActivations.Add(Sigmoid(value));
	activations->Add(inputsActivations);
	
	// Feed forward the activation values
	for (int l = 0; l < weights.Num(); l++)
	{
		TArray<float> ws; // The weighted sum for each unit in this layer
		TArray<float> a; // The activation for each unit in this layer

		TArray<float> prevActivation = (*activations)[l]; // The activations of the previous layer
		prevActivation.Add(1.0f); // Add the extra value for the bias
		for (int j = 0; j < weights[l].Num(); j++)
		{
			float z = Dot(prevActivation, weights[l][j]);
			ws.Add(z);
			a.Add(Sigmoid(z));
		}

		weightedSums->Add(ws);
		activations->Add(a);
	}

	// Return the output values
	return (*activations).Top();
}

TArray<float> UNeuralNetwork::Run(TArray<float> inputs)
{
	TArray<TArray<float>> weightedSums, activations;
	return Run(inputs, &weightedSums, &activations);
}

float UNeuralNetwork::Train(TArray<float> inputs, TArray<float> expectedOutputs)
{
	// Run the NN and get the resulted output
	TArray<TArray<float>> weightedSums, activations;
	TArray<float> outputs = Run(inputs, &weightedSums, &activations);

	// The deltas for each unit in each layer (how a change in its value affects a change in the error)
	// Note: the layers are in reverse order, i.e. the output layer has index 0
	TArray<TArray<float>> deltas;

	// Compute the deltas for the last layer
	TArray<float> outputSigmoidPrimes;
	for (float value : weightedSums.Top()) outputSigmoidPrimes.Add(SigmoidPrime(value));
	deltas.Add(Multiply(Difference(activations.Top(), expectedOutputs), outputSigmoidPrimes));

	// Compute the deltas for each layer backwards: backpropagation
	for (int l = weights.Num() - 2; l >= 0; l--)
	{
		TArray<TArray<float>> nextLayerWeightsT = Transpose(weights[l+1]);

		TArray<float> d;
		for (int i = 0; i < weights[l].Num(); i++)
		{
			d.Add(Dot(nextLayerWeightsT[i], deltas.Top()) * SigmoidPrime(weightedSums[l][i]));
		}
		deltas.Add(d);
	}

	// Reverse deltas so they can use in the same order as weights
	deltas = Reverse(deltas);

	// Alter the weights in each layer
	for (int l = 0; l < weights.Num(); l++)
	{
		for (int j = 0; j < weights[l].Num(); j++)
		{
			// Adjust the weight for each connection
			for (int i = 0; i < weights[l][j].Num() - 1; i++)
			{
				weights[l][j][i] -= deltas[l][j] * activations[l][i] * learningRate;
			}
			
			// Adjust the weight for the bias
			weights[l][j][weights[l][j].Num() - 1] -= deltas[l][j] * learningRate;
		}
	}

	// Update the learning rate
	learningRate = initialLearningRate / (1.0f + learningRateDecay * ++epoche);

	// Calculate the error made
	return ComputeError(expectedOutputs, outputs);
}


// ------ AUXILIARY MATHEMATICAL METHODS ------

float UNeuralNetwork::Dot(const TArray<float>& a, const TArray<float>& b) const
{
	if (a.Num() != b.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to dot product two vectors with non-equal dimensions: %d - %d."), a.Num(), b.Num());
		return 0.0f;
	}

	float value = 0.0f;
	for (int i = 0; i < a.Num(); i++)
	{
		value += a[i] * b[i];
	}
	return value;
}

float UNeuralNetwork::ComputeError(const TArray<float>& a, const TArray<float>& b) const
{
	if (a.Num() != b.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to compare two vectors with non-equal dimensions: %d - %d."), a.Num(), b.Num());
		return 0.0f;
	}

	float value = 0.0f;
	for (int i = 0; i < a.Num(); i++)
	{
		value += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return value * 0.5f;
}

TArray<float> UNeuralNetwork::Difference(const TArray<float>& a, const TArray<float>& b) const
{
	if (a.Num() != b.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to compute the difference of two vectors with non-equal dimensions: %d - %d."), a.Num(), b.Num());
	}

	TArray<float> diff;
	for (int i = 0; i < FMath::Min(a.Num(), b.Num()); i++)
	{
		diff.Add(a[i] - b[i]);
	}
	return diff;
}

TArray<float> UNeuralNetwork::Multiply(const TArray<float>& a, const TArray<float>& b) const
{
	if (a.Num() != b.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to multiply two vectors with non-equal dimensions: %d - %d."), a.Num(), b.Num());
	}

	TArray<float> mult;
	for (int i = 0; i < FMath::Min(a.Num(), b.Num()); i++)
	{
		mult.Add(a[i] * b[i]);
	}
	return mult;
}

TArray<TArray<float>> UNeuralNetwork::Transpose(const TArray<TArray<float>>& a) const
{
	TArray<TArray<float>> t;
	for (int i = 0; i < a[0].Num(); i++)
	{
		TArray<float> x;
		for (int j = 0; j < a.Num(); j++)
		{
			x.Add(a[j][i]);
		}
		t.Add(x);
	}
	return t;
}

template<typename T>
TArray<T> UNeuralNetwork::Reverse(const TArray<T>& a) const
{
	TArray<T> result;
	for (int i = a.Num() - 1; i >= 0; i--)
	{
		result.Add(a[i]);
	}
	return result;
}
