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

void UNeuralNetwork::Init(int inputs, int outputs, TArray<int> hiddenLayers)
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
}

TArray<float> UNeuralNetwork::Run(TArray<float> inputs)
{
	TArray<TArray<float>> activations; // The activations of each unit in each layer

	// Initialize the first layer with the input values
	activations.Add(inputs);
	activations[0].Add(1.0f); // Add the extra value for the bias
	
	// Feed forward the activation values
	for (int l = 0; l < weights.Num(); l++)
	{
		TArray<float> a;
		for (int j = 0; j < weights[l].Num(); j++)
		{
			float z = Dot(activations[l], weights[l][j]);
			a.Add(Sigmoid(z));
		}
		if (l < weights.Num() - 1) a.Add(1.0f); // Add the extra value for the bias (not needed for the output layer)
		activations.Add(a);
	}

	// Return the output values
	return activations.Top();
}

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

float UNeuralNetwork::Train(TArray<float> inputs, TArray<float> expectedOutputs)
{
	// Run the NN and get the resulted output
	TArray<float> outputs = Run(inputs);

	// Calculate the error made
	float error = ComputeError(expectedOutputs, outputs);

	return error;
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