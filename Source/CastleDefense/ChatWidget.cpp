// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "CastleDefenseGameInstance.h"
void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
	m_textToSend->OnTextCommitted.AddDynamic(this, &UChatWidget::OnChatCommitted);
}
void UChatWidget::AddMessage(FString& msg)
{
	UTextBlock* pTextBlock = NewObject<UTextBlock>(m_chatHistory);
	pTextBlock->SetText(FText::FromString(msg));

	m_chatHistory->AddChild(pTextBlock);
	m_chatHistory->ScrollToEnd();
}
void UChatWidget::FocusOnChat()
{
	UWorld* pCurWolrd = GetWorld();
	APlayerController* pController = UGameplayStatics::GetPlayerController(pCurWolrd, 0);
	if (pController)
	{
		pController->SetInputMode(FInputModeUIOnly());
		m_textToSend->SetIsEnabled(true);
		m_textToSend->SetFocus();
	}
}

void UChatWidget::OnChatCommitted(const FText& text, ETextCommit::Type commitType)
{
	if (commitType == ETextCommit::OnEnter)
	{
		UE_LOG(LogTemp, Display, TEXT("ChatComitted"));
		FText trimedText = FText::TrimPrecedingAndTrailing(text);
		APlayerController* pController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!trimedText.IsEmpty())
		{
			FString trimmedStr = trimedText.ToString();
			UCastleDefenseGameInstance* pGameInstance = 
				GetGameInstance<UCastleDefenseGameInstance>();
			int32 id  = pGameInstance->GetUserID();
			FString msgString = FString::FromInt(id) + ": " + trimmedStr;

			AddMessage(msgString);

			char* msgArray = TCHAR_TO_ANSI(*msgString);
			pGameInstance->SendMessage(msgArray);
			m_textToSend->SetText(FText());
			m_textToSend->SetIsEnabled(false);
			pController->SetInputMode(FInputModeGameOnly());
		}
	}
}
