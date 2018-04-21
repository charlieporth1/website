
var
	jqKeywordInspireInput
	,jqKeywordsInspirationList
	,laddaInspireBtn
	,isLoading
;

$(function() {  

	if ($('#keywords-inspiration-list').children().length > 0) {
		$('.navbar-nav').find('a[href="#inspirer"]').click();
	}

	jqKeywordInspireInput=$('#inspirer').find('.input').find('input[type="text"]');
	laddaInspireBtn = Ladda.create( document.querySelector( '#btn-inspire' ) );

	jqKeywordsInspirationResults=$('#inspirer').find('.keywords-compiled');
	jqKeywordsInspirationResults.autosize();


	$('#btn-clear').on('click', function() {
		clearKeywords();
	})

	$('#btn-inspire').on('click', function(e) {
		e.preventDefault(); e.stopPropagation();
		getAllKeywords();
	});


	$('#inspirer').on('keypress', 'input[type="text"]', function(e) {
		if (e.keyCode == 13) {
			getAllKeywords();
		}
	});

	$('#keywords-inspiration-list').on('click', 'a', function(e) {
		e.preventDefault(); e.stopPropagation();
		$('#inspirer').find('.keywords-compiled').append( $(this).attr('data-keyword')+'\n' );
		jqKeywordsInspirationResults.trigger('autosize.resize');
		$(this).hide(300, function () {
			$(this).remove();
		});
	});

	$('#keywords-inspiration-list').on('click', '.close', function(e) {
		e.preventDefault(); e.stopPropagation();
		$(this).parent().hide(300, function () {
			$(this).remove();
		});
	});


	$('#inspirer').on('click', '.example-keyword', function(e) {
		e.preventDefault(); e.stopPropagation();
		jqKeywordInspireInput.val( $(this).text() );
		getAllKeywords();
	});

});

function clearKeywords() {
	$('#keywords-inspiration-list').html('');
}

function getAllKeywords() {
	clearKeywords();
	isLoading=true;
	for (var ind=0;ind<5;ind++) {
		getKeywords(ind);
	}
	isLoading=false;
}

function getKeywords(ind) {
	var
		keyword=jqKeywordInspireInput.val()
	;
	laddaInspireBtn.start();
	jqKeywordInspireInput.attr('disabled', 'disabled');
	$.ajax({
		// url:"http://keywordstoaster.localhost.com/inspirer.php"
		url:"/inspirer.php"
		,method:"POST"
		,data:{
			"inspire":keyword
			,"service":ind
		}
		,success:function onDataLoaded(data) {
			jqKeywordInspireInput.removeAttr('disabled');
			if (!isLoading) laddaInspireBtn.stop();
			listKeywords(JSON.parse(data));
		}
	});
}

var 
	// keywordBtnTemplate='<a href="/?inspire={keyword}" data-keyword="{keyword}" class="btn btn-success">{keyword}<span class="close" aria-hidden="true">&times;</span></a>'
	keywordBtnTemplate='<a href="/?inspire={keyword}" data-keyword="{keyword}" class="btn btn-success">{keyword}</a>'
;
function listKeywords(data) {

	var keywordsStr='';
	$.each(data, function(index, item) {
		if (item=="") return true;

		keywordsStr+=keywordBtnTemplate.replace(/\{keyword\}/gi, item);
	});

	$('#keywords-inspiration-list').append( keywordsStr );
}

