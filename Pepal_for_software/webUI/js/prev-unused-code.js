function queryProduct() {

	var form = document.querySelector("#query");
	var prod_rfid = form[0].value;

	$.ajax({
  		type: "POST",
  		url: "http://localhost:5000/query",
  		data: {rfid: prod_rfid}
		}).done(function( o ) {
			console.log(o)
			$('#messagebox').show();
			$('#message').text("");
			$('#messageTwo').text("");

			var success = true;
			if(o['code'] == "SUCCESS") {
				product = JSON.parse(o['product']);
				producttwo = JSON.parse(o['product2']);
				var messageObject = $('#message').text("[peer1]\n RFID: " + prod_rfid  + "\n Owner: " + product['CurrOwner'] + "\n Name: " + product['ProdName']  + "\n Description: " + product['ProdStatus']);
				messageObject.html(messageObject.html().replace(/\n/g,'<br/>'));
				var messageObjectTwo = $('#messageTwo').text("[peer2]\n RFID: " + prod_rfid + "\n Owner: " + producttwo['CurrOwner']+  "\n Name: " + producttwo['ProdName'] + "\n Description: " + producttwo['ProdStatus'])
				messageObjectTwo.html(messageObjectTwo.html().replace(/\n/g,'<br/>'));

			}
			else {
				var messageObject = $('#messageTwo').text("Error Message: " + o['error'] + "\n");
				messageObject.html(messageObject.html().replace(/\n/g,'<br/>'));
				success = false;
			}

			

			if(success) {
				$('#messagebox').removeClass('alert-danger').addClass('alert-success');
			}
			else {
				$('#messagebox').removeClass('alert-success').addClass('alert-danger');
			}

			getBlocks();
	});	


}


function transferProduct() {
	var form = document.querySelector("#transfer");
	var prod_caller = form[0].value;
	var prod_rfid = form[1].value;
	var prod_newOwner = form[2].value;

	$.ajax({
  		type: "POST",
  		url: "http://localhost:5000/transferProduct",
  		data: {caller: prod_caller, rfid : prod_rfid, newOwner: prod_newOwner}
		}).done(function( o ) {
			console.log(o)
			if(o["code"] == "SUCCESS") {
				$('#messagebox').show();
				$('#message').text("Product transfer submitted.");
				$('#messageTwo').text("");
				$('#messagebox').removeClass('alert-danger').addClass('alert-success');

			}
			else {
				$('#messagebox').show();
				$('#message').text("Product not transfered. " + o['error']);
				$('#messageTwo').text("");
				$('#messagebox').removeClass('alert-success').addClass('alert-danger');

			}
			getBlocks();
	});	

}




function getBlocks() {
	blockchainDiv.innerHTML = "";
	var blocks = [];
	// Get Number of Blocks
	number = 0;
	//var blocks = [];
	$.ajax({
  		type: "GET",
  		url: "http://localhost:5000/getNumBlocks",
		}).done(function( num ) {
   			number = num;
   			// Print Blocks
   			document.getElementById('blockchain').innerHTML = " ";
   			for(var i = 0; i < number; i++) {
				$.ajax({
  					type: "POST",
  					url: "http://localhost:5000/getBlock",
  					data: {num : i}
					}).done(function( o ) {
						
						var content = document.querySelector('#blockTemplate').content;
   					 	content.querySelector('#block-number').textContent = o['num'];
   					 	short_hash = o['currentHash'].substring(0,48) + "  ..."
   					 	content.querySelector('#block-blockHash').textContent = short_hash;
   					 	content.querySelector('#block-time').textContent = o['time']
					 
   					 	content.querySelector('#txs').textContent = "";


						if(o.hasOwnProperty('transactions')) {

							for(index = 0; index < o['transactions'].length; index++) {
								var transaction = o['transactions'][index];
								var whole_decoded_payload = atob(transaction['payload']);
								var decoded_payload = whole_decoded_payload.replace("BroleBaccount", " ");
								var payload_array = decoded_payload.split('\n');
								
								var desc = "";
								
								if(payload_array[3].toString().localeCompare("init") == 0) {
									var init_payload = decoded_payload.split("init");
									desc = "init" + init_payload[1]
								}

								else {
									var split = decoded_payload.split(payload_array[2].toString());
									desc = payload_array[2].toString() + split[1];
									desc = desc + "<br>" + "<u> Signature: </u>" + transaction['signature'].substring(0,48) + "  ...";;
									desc = desc + "<br>" +"<u> Certificate: </u> " + transaction['cert'].substring(0,48) + "  ...";
								}

								var tx = document.querySelector('#tx-template').content;
								tx.querySelector("#tx-head").textContent = index;
								tx.querySelector("#tx-desc").innerHTML = desc;

								content.querySelector('#txs').appendChild(document.importNode(tx, true));

							}

						}
						else {
							var tx = document.querySelector('#tx-template').content;
							tx.querySelector("#tx-head").textContent = ""
							tx.querySelector("#tx-desc").textContent = "No transactions";

							content.querySelector('#txs').appendChild(document.importNode(tx, true));
						}
						blockchainDiv.appendChild(document.importNode(content, true));


						



				});	
				

			}
			
			$.ajax({
  					type: "POST",
					url: "http://localhost:5000/allOwnerProducts",
					data: {owner : 'factory'}
					}).done(function( o ) {
						console.log(o);
					   	$('#factory-panel-left').text("");
					   	//$('#factory-panel-right').text("");
					   	for(var i =0; i < o.length;i++) {
					   		var leftPrev = $('#factory-panel-left').html();
					   		//var rightPrev = $('#factory-panel-right').html();

					   		$('#factory-panel-left').html(leftPrev + o[i][0] + "<br>");
					   		//$('#factory-panel-right').html(rightPrev + o[i][1] + "<br>");

					    }
			});

			$.ajax({
  					type: "POST",
					url: "http://localhost:5000/allOwnerProducts",
					data: {owner : 'warehouse'}
					}).done(function( o ) {
						console.log(o);
						$('#warehouse-panel-left').text("");
					   	//$('#warehouse-panel-right').text("");
					   for(var i =0; i < o.length;i++) {
					   		
					   		var leftPrev = $('#warehouse-panel-left').html();
					   		//var rightPrev = $('#warehouse-panel-right').html();

					   		$('#warehouse-panel-left').html(leftPrev + o[i][0] + "<br>");
					   		//$('#warehouse-panel-right').html(rightPrev + o[i][1] + "<br>");

					    }
			});		

			$.ajax({
  					type: "POST",
					url: "http://localhost:5000/allOwnerProducts",
					data: {owner : 'retailer1'}
					}).done(function( o ) {
						console.log(o);
						$('#retailer1-panel-left').text("");
					   	//$('#retailer1-panel-right').text("");
					   for(var i =0; i < o.length;i++) {
					   		var leftPrev = $('#retailer1-panel-left').html();
					   		//var rightPrev = $('#retailer1-panel-right').html();

					   		$('#retailer1-panel-left').html(leftPrev + o[i][0] + "<br>");
					   		//$('#retailer1-panel-right').html(rightPrev + o[i][1] + "<br>");

					    }
			});
			$.ajax({
  					type: "POST",
					url: "http://localhost:5000/allOwnerProducts",
					data: {owner : 'retailer2'}
					}).done(function( o ) {
						console.log(o);
						$('#retailer2-panel-left').text("");
					   	//$('#retailer2-panel-right').text("");

					   	for(var i =0; i < o.length;i++) {

					   		var leftPrev = $('#retailer2-panel-left').html();
					   		//var rightPrev = $('#retailer2-panel-right').html();

					   		$('#retailer2-panel-left').html(leftPrev + o[i][0] + "<br>");
					   		//$('#retailer2-panel-right').html(rightPrev + o[i][1] + "<br>");
					    }
			});			
			
	});	
	
}


function createProduct() {
	var form = document.querySelector("#create");
	var prod_rfid = form[0].value;
	var prod_name = form[1].value;
	var prod_desc = form[2].value;

	$.ajax({
  		type: "POST",
  		url: "http://localhost:5000/create",
  		data: {rfid : prod_rfid, name: prod_name, desc: prod_desc}
		}).done(function( o ) {
			console.log(o);
			if(o["code"] == "SUCCESS") {
				$('#messagebox').show();
				$('#message').text("Transaction submitted.");
				$('#messageTwo').text("");
				$('#messagebox').removeClass('alert-danger').addClass('alert-success');

			}
			else {
				$('#messagebox').show();
				$('#message').text("Transaction not submitted. " + o['error']);
				$('#messageTwo').text("");
				$('#messagebox').removeClass('alert-success').addClass('alert-danger');

			}

			getBlocks();
	});	

}


function clear_all_selects() {
	var all_select_ids = ['#car_entity_val', '#supplier_entity_val'];
	for(var i = 0; i < all_select_ids.length; i++) {
		if($(all_select_ids[i]).length) {$(all_select_ids[i]).css('display','none')};
	}
}


/*$('#options').querySelector('#Car').click(function() {
	document.write('Yo')
 	clear_all_selects();
 	$('#car_entity_val').toggle();
});

$('.supplier_option').click(function() {
 	clear_all_selects();
 	$('#supplier_entity_val').toggle();
});*/

// Get blockchain at the start
//getBlocks();

function validateToken(role, entity, token) {
	if (role == "Car") {
		for (var i=0; i < registered_car.length; i++){
			if (registered_car[i] == entity) {
				if (registered_car_token[i] == token) {
					return true;
				}
			}
		}
	}
	else if (role == "Supplier") {
		for (var i = 0; i<registered_supplier.length; i++){
			if (registered_supplier[i] == entity) {
				if (registered_supplier_token[i] == token) {
					return true;
				}
			}
		}
	}
	else if (role == "Manufacturer") {
		for (var i=0; i < registered_manufacturer.length; i++){
			if (registered_manufacturer[i] == entity) {
				if (registered_manufacturer_token[i] == token) {
					return true;
				}
			}
		}
	}
	else if (role == "Dealer") {
		for (var i=0; i < registered_dealer.length; i++){
			if (registered_dealer[i] == entity) {
				if (registered_dealer_token[i] == token) {
					return true;
				}
			}
		}
	}
	else if (role == "Repair") {
		for (var i=0; i < registered_shop.length; i++){
			if (registered_shop[i] == entity) {
				if (registered_shop_token[i] == token) {
					return true;
				}
			}
		}
	}
	
	return false;
}