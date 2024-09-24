// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

/**
 * 
 */
class UScrollBox;
class UEditableText;
UCLASS()
class CASTLEDEFENSE_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void AddMessage(FString& msg);
	void FocusOnChat();
private:
	UFUNCTION()
	void OnChatCommitted(const FText& text, ETextCommit::Type type);
private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* m_chatHistory;
	UPROPERTY(meta = (BindWidget))
	UEditableText* m_textToSend;
};
