void disable_callbacks(POLD_CALLBACKS oldCallbacks)
{
	POBJECT_TYPE procType = *PsProcessType;
	if (procType && MmIsAddressValid((void*)procType))
	{
		__try
		{
			QWORD callbackListOffset = GetCallbackListOffset();
			if (callbackListOffset && MmIsAddressValid((void*)((QWORD)procType + callbackListOffset)))
			{
				LIST_ENTRY* callbackList = (LIST_ENTRY*)((QWORD)procType + callbackListOffset);
				if (callbackList->Flink && MmIsAddressValid((void*)callbackList->Flink))
				{
					CALLBACK_ENTRY_ITEM* firstCallback = (CALLBACK_ENTRY_ITEM*)callbackList->Flink;
					CALLBACK_ENTRY_ITEM* curCallback = firstCallback;

					do
					{
						if (curCallback && MmIsAddressValid((void*)curCallback) && MmIsAddressValid((void*)curCallback->CallbackEntry))
						{

							ANSI_STRING altitudeAnsi = { 0 };
							UNICODE_STRING altitudeUni = curCallback->CallbackEntry->Altitude;
							RtlUnicodeStringToAnsiString(&altitudeAnsi, &altitudeUni, 1);
							
							if (!strcmp(altitudeAnsi.Buffer, "199285")) continue;

							if (1) 
							{
								if (curCallback->PreOperation) {
									oldCallbacks->PreOperationProc = (QWORD)curCallback->PreOperation;
									curCallback->PreOperation = DummyObjectPreCallback;
								}
								if (curCallback->PostOperation) {
									oldCallbacks->PostOperationProc = (QWORD)curCallback->PostOperation;
									curCallback->PostOperation = DummyObjectPostCallback;
								}
								RtlFreeAnsiString(&altitudeAnsi);
								break;
							}

							RtlFreeAnsiString(&altitudeAnsi);
						}

						curCallback = curCallback->CallbackList.Flink;
					} while (curCallback != firstCallback);
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}

	POBJECT_TYPE threadType = *PsThreadType;
	if (threadType && MmIsAddressValid((void*)threadType)) {
		__try {
			QWORD callbackListOffset = GetCallbackListOffset();
			if (callbackListOffset && MmIsAddressValid((void*)((QWORD)threadType + callbackListOffset))) {
				LIST_ENTRY* callbackList = (LIST_ENTRY*)((QWORD)threadType + callbackListOffset);
				if (callbackList->Flink && MmIsAddressValid((void*)callbackList->Flink)) {
					CALLBACK_ENTRY_ITEM* firstCallback = (CALLBACK_ENTRY_ITEM*)callbackList->Flink;
					CALLBACK_ENTRY_ITEM* curCallback = firstCallback;

					do
					{
						if (curCallback && MmIsAddressValid((void*)curCallback) && MmIsAddressValid((void*)curCallback->CallbackEntry))
						{
							ANSI_STRING altitudeAnsi = { 0 };
							UNICODE_STRING altitudeUni = curCallback->CallbackEntry->Altitude;
							RtlUnicodeStringToAnsiString(&altitudeAnsi, &altitudeUni, 1);
							
							if (!strcmp(altitudeAnsi.Buffer, "199285")) continue;

							if (1)
							{
								if (curCallback->PreOperation)
								{
									oldCallbacks->PreOperationThread = (QWORD)curCallback->PreOperation;
									curCallback->PreOperation = DummyObjectPreCallback;
								}
								if (curCallback->PostOperation)
								{
									oldCallbacks->PostOperationThread = (QWORD)curCallback->PostOperation;
									curCallback->PostOperation = DummyObjectPostCallback;
								}
								RtlFreeAnsiString(&altitudeAnsi);
								break;
							}

							RtlFreeAnsiString(&altitudeAnsi);
						}
						curCallback = curCallback->CallbackList.Flink;
					} while (curCallback != firstCallback);
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}
}
UINT64 FindPspLoadImageNotifyRoutine()
{
	UNICODE_STRING usFunc = { 0 };
	RtlInitUnicodeString(&usFunc, L"PsSetLoadImageNotifyRoutine");
	UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

	if (!Function)
		return 0;

	char Pattern[3] = { 0x48,0x8D,0x0D };
	Function = FindPattern(Function, 0xFF, Pattern, sizeof(Pattern));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 3), 4);
		Function = Function + 7 + Offset;
	}

	return Function;
}
NTSTATUS DisablePsImageCallback()
{
	UINT64 PspLoadImageFunc = FindPspLoadImageNotifyRoutine();
	UINT64 final=0;
	if (PspLoadImageFunc)
	{
		ListCallbacks(PspLoadImageFunc, DriverInfo.Base, DriverInfo.Size, &final, 1);
		return PsRemoveLoadImageNotifyRoutine(final);
	}
	return STATUS_UNSUCCESSFUL;
}
UINT64 FindPspCreateThreadNotifyRoutine()
{
	UNICODE_STRING usFunc = { 0 };
	RtlInitUnicodeString(&usFunc, L"PsSetCreateThreadNotifyRoutine");
	UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

	if (!Function) return 0;
	
	char Pattern2[3] = { 0x48,0x8D,0x0D };
	Function = FindPattern(Function, 0xFF, Pattern2, sizeof(Pattern2));
	if (Function)
	{
		LONG Offset = 0;
		memcpy(&Offset, (UCHAR*)(Function + 3), 4);
		UINT64 Addy = Function + 7 + Offset;
		return Addy;
	}

	return 0;
}
NTSTATUS DisablePsThreadCallback()
{
	UINT64 PspCreateThreadFunc = FindPspCreateThreadNotifyRoutine();
	UINT64 final = 0;
	if (PspCreateThreadFunc)
	{
		ListCallbacks(PspCreateThreadFunc, DriverInfo.Base, DriverInfo.Size, &final, 1);
		return PsRemoveCreateThreadNotifyRoutine(final);
	}
	return STATUS_UNSUCCESSFUL;
}
