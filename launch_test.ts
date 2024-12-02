#!/usr/bin/env -S npx tsx

//Launches all tests located in test_files/ with all combination

import * as fs from 'fs';
import * as path from 'path';
import { execSync} from 'child_process';

let combinations:string[] = [];
function generate_combination(options: string[]):void {
  const result: string[][] = [];

  const max_length = options.length;
  if (max_length == 0) {
    return;
  }
  let first_char : string = options[0];

  //pop first element
  options.shift();
  let char: string;
  for ( char of options) {
      combinations.push(first_char + char);
  }

  generate_combination(options);
  return;
}

generate_combination(["a", "g", "u", "r", "p"]);  

const __dirname = path.dirname(new URL(import.meta.url).pathname);
const testPath = path.join(__dirname, 'test_files');
const nmPath = path.join(__dirname, "nm");
const resultPath = path.join(__dirname, "test_res");
const test_files = fs.readdirSync(testPath);
let i = 0;
for (let test of test_files) {
  for (let combination of combinations) {
    let flag = "-" + combination;
    execSync(`mkdir -p ${resultPath}/${i}`);
    execSync(`${nmPath} ${flag} ${testPath}/${test} > ${resultPath}/${i}/${combination}`);
  }
  i++;
}