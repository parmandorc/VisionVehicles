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
			for (int i = 0; i < dimensions[l - 1]; i++)
			{
				w.Add(FMath::RandRange(0.0f, 1.0f));
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

	// Feed forward the activation values
	for (int l = 0; l < weights.Num() - 1; l++)
	{
		TArray<float> a;
		for (int j = 0; j < weights[l].Num(); j++)
		{
			float z = Dot(activations[l], weights[l][j]);
			a.Add(Sigmoid(z));
		}
		activations.Add(a);
	}

	// Return the output values
	return activations.Top();
}

float UNeuralNetwork::Dot(TArray<float>& a, TArray<float>& b)
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