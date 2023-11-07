// Decode uplink function.
//
// Input is an object with the following fields:
// - bytes = Byte array containing the uplink payload, e.g. [255, 230, 255, 0]
// - fPort = Uplink fPort.
// - variables = Object containing the configured device variables.
//
// Output must be an object with the following fields:
// - data = Object representing the decoded payload.
function decodeUplink(input) {
  
  bytes = input.bytes;
  
  function bytesToFloat(bytes) {
    	var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
    	var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    	var e = bits>>>23 & 0xff;
    	var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    	var f = sign * m * Math.pow(2, e - 150);
    	return f;
	}
  
  function bytesToInt16(bytes) {
    	var result = bytes[1]*256 + bytes[0];
    	return result;
  }
  
  return {
    data: {
      temperature: +bytesToFloat(bytes.slice(0,4)),
      hum: +bytesToFloat(bytes.slice(4,8)),
      pm10: +bytesToInt16(bytes.slice(8,10)),
		  pm25: +bytesToInt16(bytes.slice(10,12)),
    }
  };
}

// Encode downlink function.
//
// Input is an object with the following fields:
// - data = Object representing the payload that must be encoded.
// - variables = Object containing the configured device variables.
//
// Output must be an object with the following fields:
// - bytes = Byte array containing the downlink payload.
function encodeDownlink(input) {
  return {
    bytes: [225, 230, 255, 0]
  };
}
