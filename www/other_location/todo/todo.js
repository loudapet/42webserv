
window.onload = function() {
function newToDo() {
		const todoText = prompt("Enter a new TO-DO:");
		if (todoText) {
			const todoDiv = document.createElement("div");
			todoDiv.textContent = todoText;
			todoDiv.addEventListener("click", delToDo);
			document.getElementById("ft_list").prepend(todoDiv);
			saveToCookie();
		}
	}
	
	function delToDo() {
		const shouldRemove = confirm("Do you want to remove this TO-DO?");
		if (shouldRemove) {
			this.remove();
			saveToCookie();
		}
	}
	
	function saveToCookie() {
		const todos = [];
		const todoElements = document.querySelectorAll("#ft_list > div");
		for (const todo of todoElements) {
			todos.push(todo.textContent);
		}
		console.log(todos);
		const todoListString = JSON.stringify(todos);
		console.log(todoListString );
		document.cookie = `todoList=${todoListString}; max-age=31536000; SameSite=None; Secure; path=/`;
	}
	
	function loadFromCookie() {
		//removing the todolist from the cookie
		const cookieValue = document.cookie.substring(9);

		console.log(document.cookie);
		console.log(cookieValue);
		if (cookieValue) {
			const todos = JSON.parse(cookieValue);
			for (const todoText of todos) {
				const todoDiv = document.createElement("div");
				todoDiv.textContent = todoText;
				todoDiv.addEventListener("click", delToDo);
				document.getElementById("ft_list").append(todoDiv);
			}
		}
	}
	
	document.getElementById("newButton").addEventListener("click", newToDo);
	
	loadFromCookie();
}