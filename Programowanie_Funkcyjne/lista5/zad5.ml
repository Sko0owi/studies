type 'a dllist = 'a dllist_data Lazy.t
(* Note: lazy_t is the built-in type constructor used by the compiler for the lazy keyword. You should not use it directly. Always use Lazy.t instead. *)
and 'a dllist_data ={ elem : 'a; next : 'a dllist; prev : 'a dllist;  }

let prev (prev : 'a dllist) = 
  match prev with
| lazy {prev; elem; next} -> prev


let next = function
  | lazy {prev; elem; next} -> next

let elem (prev : 'a dllist) = 
  match prev with
  | lazy {prev; elem; next} -> elem



let singleton x =
  let rec s = lazy {next = s; elem = x; prev = s} in
  s

let rec generate_dlist (prev : 'a dllist) (xs : 'a list) (first : 'a dllist) =
  match xs with
  | [] -> first, prev
  | x :: xs -> 
    let last = ref (singleton x) in
    let rec current = lazy begin
        let (next_pointer,last_remebered) = generate_dlist current xs first in
          last := last_remebered; 
          {prev = prev; elem = x; next = next_pointer} 
    end in
      let result = (Lazy.force current) in 
        lazy result, !last 

let rec of_list xs =
  match xs with
  | [] -> failwith "puuusto"
  | x :: xs -> 
    let rec first = lazy begin
      let (next_pointer,last_remebered) = generate_dlist first xs first in
      {prev = last_remebered; elem = x; next = next_pointer}
    end in first

let x = (of_list [1 ; 2 ; 3;]);;

assert ((prev (next x)) == (next (prev x)));;

let rec gen_prev xs =
  let rec current = lazy {prev = gen_prev current ; elem = ((elem xs) - 1) ; next = xs} in current
let rec gen_next xs =
  let rec current = lazy {prev = xs ; elem = ((elem xs) + 1) ; next = gen_next current} in current

let rec integers = lazy {prev = gen_prev integers; elem = 0; next = gen_next integers} 

let test1 = elem (next integers)
let test2 = elem (next (next integers))
let test3 = elem (prev integers)
let test4 = elem (prev (prev integers))