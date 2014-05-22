void ScriptMessageCallback(const asSMessageInfo *msg, void *param){

	//wstring type = L"";

	if(msg->type == asMSGTYPE_WARNING){
		//type = L"script [WARNING]";
		Logger::Get()->Info(L"[SCRIPT] [WARNING] "+Convert::CharPtrToWstring(msg->section)+L" ("+Convert::ToWstring(msg->row)+L", "+Convert::ToWstring(msg->col)+L") : "+Convert::ToWstring(msg->message), true);
	} else if(msg->type == asMSGTYPE_INFORMATION){
		//type = L"script [INFO]";
		Logger::Get()->Info(L"[SCRIPT] "+Convert::CharPtrToWstring(msg->section)+L" ("+Convert::ToWstring(msg->row)+L", "+Convert::ToWstring(msg->col)+L") : "+Convert::ToWstring(msg->message), true);
	} else {
		//type = L"script [ERROR]";
		Logger::Get()->Error(L"[SCRIPT] "+Convert::CharPtrToWstring(msg->section)+L" ("+Convert::ToWstring(msg->row)+L", "+Convert::ToWstring(msg->col)+L") : "+Convert::ToWstring(msg->message), true);
	}
	
	return;
	//printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}