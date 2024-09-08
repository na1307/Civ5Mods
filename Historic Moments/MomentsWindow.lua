function Show()
	ContextPtr:SetHide(false)
end

function Hide()
	ContextPtr:SetHide(true)
end

function OnCloseButton()
	Hide()
end
Controls.CloseButton:RegisterCallback(Mouse.eLClick, OnCloseButton)

function AddDropdownEntry(additionalEntries)
	table.insert(additionalEntries, { text = Locale.ConvertTextKey("TXT_KEY_CITY"), call = Show });
end
LuaEvents.AdditionalInformationDropdownGatherEntries.Add(AddDropdownEntry);
LuaEvents.RequestRefreshAdditionalInformationDropdownEntries();

Hide();