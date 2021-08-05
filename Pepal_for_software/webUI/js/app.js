/////Global variables
blockchainDiv = document.getElementById('blockchain')
registered_user = false //flag to indicate whether user is registered

var current_token;
var current_airbags = []
var current_cars = []

registered_users = {}	// Dictionary of all current registered users. Key is the role

var IP_ADDRESS = "localhost"

function fillCredentialsFields(strRole, strEntity) {
	divs = document.getElementsByClassName("userCredentials");
	for(var i=0; i < divs.length; i++) {
		divs[i].innerHTML = "Credentials: (Role, " + strRole + "), (Entity, " + strEntity + ")" ;
	}
	
}


function fillInternalCredentials(strRole, strEntity, token) {

	if(registered_users[strRole]) {
		registered_users[strRole].push([strEntity,token]);
	}
	else {
		registered_users[strRole] = [[strEntity,token]];
	}

}


function loginUser(form_id) {
	form = document.querySelector('#'+form_id)
	myformrole = form.querySelector("#role");
	var strRole = myformrole.options[myformrole.selectedIndex].value;
	
	myformentity = form.querySelector("#entity")
	var strEntity = myformentity.options[myformentity.selectedIndex].value;
	
	fillCredentialsFields(strRole, strEntity);
	users_from_role = registered_users[strRole];
	for(var index in users_from_role ) {
		if(users_from_role[index][0] == strEntity) {
			current_token = users_from_role[index][1];
		}
	}
	console.log(strRole + ' ' + strEntity )
	console.log(current_token);
	$("#messagebox").text('User switched to Role: ' + strRole + ' Entity: ' + strEntity);
}


function sendRegisterRequest(strRole, strEntity, switchUser) {
	console.log(strRole + ' ' + strEntity)
	$.ajax({
  		type: "POST",
  		url: "http://"+ IP_ADDRESS + ":4000/users",
  		headers : {
				'content-type' : 'application/x-www-form-urlencoded'
		},
  		data: "username=Ford."+strRole+'.'+strEntity+"&orgName=org1"
		}).done(function( o ) {
			var resultBox = document.getElementById("#messagebox");
			if (o["success"]) {		
				console.log("token received");
				if(switchUser) {
					registered_user = true
					fillCredentialsFields(strRole, strEntity)
					console.log(o["token"]);
					current_token = o["token"]
				}
				fillInternalCredentials(strRole, strEntity, o["token"])
				
			}
			else {
				console.log("Registration Error")
			}
	});	
}


function downloadSoftware() {
	downloadform = document.querySelector('#download');
	var senderPub = downloadform.querySelector("#senderPublicKey").value;
	var recipientPub = downloadform.querySelector("#recipientPublicKey").value;
	var amount = downloadform.querySelector("#amount").value;
	var date = downloadform.querySelector("#date").value;
	var ip = downloadform.querySelector("#ip").value;
	var port = downloadform.querySelector("#port").value;

	
	if (registered_user) {
		$.ajax({
			type: "POST",
			url: "http://"+ IP_ADDRESS + ':4000/channels/mychannel/chaincodes/mycc/',
			headers : {
				'authorization': 'Bearer ' + current_token, 
				'content-type': 'application/json'
			},
			data: JSON.stringify({"fcn": "download_request", "args" : [senderPub, recipientPub, amount,date,ip,port]})
			}).done(function( o ) {
				console.log(o);
				if(o["status"] == 200){
					var str = String.fromCharCode.apply(null, o['payload']['data']);
 					console.log(str);
 					
                    $("#messagebox").text("Success: " + str); // Format MessageBox later                    }
                    
				}
				else {
					$("#messagebox").text(o.message);
				}	
				refreshBlockchain();
		});
	}
	else {
		resultBox.value = "You need to log in first!";
	}		


}


function submitPenalty() {
	penaltyForm = document.querySelector('#penalty');
	var senderSig = penaltyForm.querySelector("#senderSig").value;
	var recipientSig = penaltyForm.querySelector("#recipientSig").value;
	var message = penaltyForm.querySelector("#penaltyMessage").value;
	console.log(recipientSig + '  ' + senderSig);
	if (registered_user) {
		$.ajax({
			type: "POST",
			url: "http://"+ IP_ADDRESS + ':4000/channels/mychannel/chaincodes/mycc/',
			headers : {
				'authorization': 'Bearer ' + current_token, 
				'content-type': 'application/json'
			},
			data: JSON.stringify({"fcn": "penalty", "args" : ["penalty", message, senderSig, recipientSig]})
			}).done(function( o ) {
				console.log(o);
				if(o["status"] == 200){
					var str = String.fromCharCode.apply(null, o['payload']['data']);
 					console.log(str);
 					
                    $("#messagebox").text("Success: " + str); // Format MessageBox later
                    
				}
				else {
					$("#messagebox").text(o.message);
				}
				refreshBlockchain();
				
		});
	}
	else {
		resultBox.value = "You need to log in first!";
	}		


}

function returnJSON(byteArray) {
	var result = "";
	for(var index = 0; index < byteArray.length; index++) {
		result += String.fromCharCode(byteArray[index]);
	}
	return JSON.parse(result);
}

function getBlock(blocknum, total_num) {
	if(blocknum >= total_num) return;
	$.ajax({
		type: "GET",
		url: "http://"+ IP_ADDRESS + ":4000/channels/mychannel/blocks/" + blocknum +"?peer=peer1",
		headers : {
			'authorization': 'Bearer ' + current_token, 
			'content-type': 'application/json'
		}
		}).done(function( o ) {
			//$("#messagebox").text(o);
			console.log(o)
			var content = document.querySelector('#blockTemplate').cloneNode(true).content;
			
			content.querySelector('#block-number').textContent = o['header']['number']['low'];

			header_hash = o['header']['data_hash'];
			
			content.querySelector('#txs').textContent = "";

			// Loop over tx in block
			num_transactions =  o['data']['data'].length;
			for(var j=0; j < num_transactions; j++) {
				var tx = document.querySelector('#tx-template').content;
				//tx.querySelector("#tx-head").textContent = j;

				var tx_data = o['data']['data'][j];
				var tx_timestamp = tx_data['payload']['header']['channel_header']['timestamp'];
				var tx_id = tx_data['payload']['header']['channel_header']['tx_id'];
				var certificate = tx_data['payload']['header']['signature_header']['creator']['IdBytes'];
				

				try {
					var attributes = tx_data['payload']['data']['actions'][0]['payload']
					var input_arr = attributes['chaincode_proposal_payload']['input']['data']
					var input_str = ''
					for(var k=0; k < input_arr.length; k++) {
						char_code = parseInt(input_arr[k]);
						if(char_code >= 32 && char_code <=126 ) {
							input_str += String.fromCharCode(char_code);
						}
						else {
							input_str += ' ';
						}
					}
					input_str2 = '';
					var response = attributes['action']['proposal_response_payload']['extension']['response']['payload'];
					for(var k=0; k < response.length; k++) {
						char_code = parseInt(response[k]);
						if(char_code >= 32 && char_code <=126 ) {
							input_str2 += String.fromCharCode(char_code);
						}
						else {
							input_str2 += ' ';
						}
					}

					//tx.querySelector("#tx-id").innerHTML = 'Id: ' + tx_id.substring(0,18) + " ..";;
					tx.querySelector("#tx-desc").innerHTML = 'Op:' + input_str.replace('.','').replace('-','').split('mycc')[1] + '<br>' + input_str2;
				}
				catch(e) {
					tx.querySelector("#tx-head").textContent = '';
					
					if(blocknum == 0) {
						tx.querySelector("#tx-desc").innerHTML = 'Genesis Block';
					}
					else {
						tx.querySelector("#tx-desc").innerHTML = 'No Transactions';
					}

				}
			}	
				
			if(blocknum == 1) {
				tx.querySelector("#tx-head").textContent = ''
				tx.querySelector("#tx-id").innerHTML = '';
				tx.querySelector("#tx-desc").innerHTML = 'Blockchain Initialization';
			}

			content.querySelector('#txs').appendChild(document.importNode(tx, true));

			blockchainDiv.appendChild(document.importNode(content, true));
			content = '';

			getBlock(blocknum + 1, total_num);
	});

}


function refreshBlockchain() {
		blockchainDiv.innerHTML = '';
		//First get the number of blocks
		var number_of_blocks;
		$.ajax({
			type: "GET",
			url: "http://"+ IP_ADDRESS + ":4000/channels/mychannel?peer=peer1",
			headers : {
				'authorization': 'Bearer ' + current_token, 
				'content-type': 'application/json'
			}
			}).done(function( o ) {
				//$("#messagebox").text(o);
				//Get the number of blocks from o. It is in the variable height
				number_of_blocks = parseInt(o['height']['low'])
				console.log(number_of_blocks);
				
				// Recursively calls getBlock for all blocks instead of using a for loop to 
				// make sure blocks get added in right order (Gets around async Ajax without making async:false and locking up the browser)		
				getBlock(0, number_of_blocks);

		});
	
		

}











// Menu Button Forms

function clear_all_forms() {
	var all_form_ids = ['#home', '#login', '#download', 'penalty'];
	for(var i = 0; i < all_form_ids.length; i++) {
		if($(all_form_ids[i]).length) {$(all_form_ids[i]).css('display','none')};
	}
}

function home() { clear_all_forms(); $('#home').toggle(); refreshBlockchain();}
function login() { clear_all_forms(); $('#login').toggle(); $('#messagebox').text('') }
function downloadButton() { clear_all_forms(); $('#download').toggle(); $('#messagebox').text('')}
function penaltyButton() { clear_all_forms(); $('#penalty').toggle(); $('#messagebox').text('')}



function populate_airbags() {

	var selects = document.getElementsByClassName("airbag_id");
	for(var i=0; i < selects.length; i++) {
		airbags_list = ["-- select airbag here --"].concat(current_airbags);
		selected_entity = selects[i];
		selected_entity.innerHTML = '';
		for (var index in airbags_list) {
			airbag = airbags_list[index];
			newOption = document.createElement("option");
			newOption.value = airbag;
			newOption.innerHTML = airbag;
			selected_entity.options.add(newOption);
		}
	}
}

function populate_cars() {

	var selects = document.getElementsByClassName("car_id");
	for(var i=0; i < selects.length; i++) {
		var car_list = ["-- select car here --"].concat(current_cars);
		selected_entity = selects[i];
		selected_entity.innerHTML = '';
		for (var index in car_list) {
			var car = car_list[index];
			newOption = document.createElement("option");
			newOption.value = car;
			newOption.innerHTML = car;
			selected_entity.options.add(newOption);
		}
	}
}

function populate(form_id, id_selected_role, id_selected_entity) {
	form = document.querySelector('#' + form_id)
	var selected_role = form.querySelector('#'+id_selected_role);
	var selected_entity = form.querySelector('#' + id_selected_entity);
	selected_entity.innerHTML = "";

	users_from_role = [["-- select entity here --"]].concat(registered_users[selected_role.value]);
	
	for(var index in users_from_role) {
		entity = users_from_role[index][0];
		newOption = document.createElement("option");
		newOption.value = entity;
		newOption.innerHTML = entity;
		selected_entity.options.add(newOption);
	
	}
	
}





$('document').ready(function() {
	// Register new users at the start
	roles = ["Owner", "User"];
	for(var index in roles) {
		role = roles[index];
		if(role != "Car") {
			sendRegisterRequest(role, role+"1", true);
			sendRegisterRequest(role, role+"2", true);
		}
				
	}
	home();
});
