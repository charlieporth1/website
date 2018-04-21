var gajsdExistinOnError = window.onerror;
window.onerror = function myErrorHandler(errMsg, url, ln) {
	try {
		_gaq.push(['_trackEvent', 'Error', url, 'Line ' + ln + ': ' + errMsg]);
	} catch (err) {}
		if (gajsdExistinOnError) gajsdExistinOnError.call(errMsg, url, ln);
}	
